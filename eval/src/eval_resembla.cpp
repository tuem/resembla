/*
Resembla: Word-based Japanese similar sentence search library
https://github.com/tuem/resembla

Copyright 2017 Takashi Uemura

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <time.h>

#include <simstring/simstring.h>
#include <paramset.hpp>

#include "resembla_util.hpp"

#include "measure/asis_sequence_builder.hpp"
#include "measure/word_sequence_builder.hpp"
#include "measure/pronunciation_sequence_builder.hpp"
#include "measure/romaji_sequence_builder.hpp"
#include "measure/weighted_sequence_builder.hpp"
#include "measure/keyword_match_preprocessor.hpp"

#include "measure/feature_match_weight.hpp"
#include "measure/surface_match_cost.hpp"
#include "measure/romaji_weight.hpp"
#include "measure/romaji_match_cost.hpp"

#include "regression/extractor/feature_extractor.hpp"
#include "regression/extractor/regex_feature_extractor.hpp"
#include "regression/extractor/date_period_feature_extractor.hpp"
#include "regression/extractor/time_period_feature_extractor.hpp"

#include "measure/weighted_sequence_serializer.hpp"

using namespace resembla;

// list of {true_text, list of {input_text, freq}}
using TestData = std::vector<std::pair<std::wstring, std::vector<std::pair<std::wstring, int>>>>;

const string_type DELIMITER = L"\t";
const string_type WORD_FREQ_SEPARATOR = L"/";

// generates SimString index and test data
template<typename Preprocessor>
TestData prepare_data(std::string test_data_path, std::string db_path, std::string inverse_path,
        int simstring_ngram_unit, Preprocessor preprocess)
{
    simstring::ngram_generator gen(simstring_ngram_unit, false);
    simstring::writer_base<std::wstring> dbw(gen, db_path);
    std::unordered_map<std::wstring, std::unordered_set<std::wstring>> inverse;

    TestData test_data;
    bool first = true;
    std::wifstream wifs(test_data_path);
    while(wifs.good()){
        // format: {true}\t{input0}/{freq0}\t{input1}\t{freq1}...
        std::wstring line;
        std::getline(wifs, line);
        if(first){
            first = false;
            continue;
        }
        if(wifs.eof() || line.length() == 0){
            break;
        }
        size_t start = 0;
        bool first = true;
        while(start < line.size()){
            size_t end = line.find(DELIMITER, start);
            if(first){
                std::wstring original(line, start, end == std::wstring::npos ? line.size() : end - start);
                auto s = preprocess.index(original);
                if(inverse.count(s) == 0){
                    dbw.insert(s);
                    inverse[s] = {original};
                }
                else{
                    inverse[s].insert(original);
                }
                test_data.push_back(std::make_pair(original, std::vector<std::pair<std::wstring, int>>()));
                first = false;
            }
            else{
                std::wstring s;
                int f = 0;
                size_t sep = line.find(WORD_FREQ_SEPARATOR, start);
                if(sep != std::wstring::npos && (end == std::wstring::npos || sep < end)){
                    s = std::wstring(line, start, sep - start);
                    f = std::stoi(std::wstring(line, sep + 1, end == std::wstring::npos ? line.size() : end - start));
                }
                else{
                    s = std::wstring(line, start, end == std::wstring::npos ? line.size() : end - start);
                }
                test_data.back().second.push_back(std::make_pair(s, f));
            }

            if(end == std::wstring::npos){
                break;
            }
            start = end + 1;
        }
    }
    dbw.close();
    std::wofstream wofs;
    wofs.open(inverse_path);
    for(auto p: inverse){
        for(auto original: p.second){
            wofs << p.first << L'\t' << original << std::endl;
        }
    }
    return test_data;
}

int main(int argc, char* argv[])
{
    std::vector<std::pair<std::chrono::system_clock::time_point, std::string>> time_points;
    time_points.push_back(std::make_pair(std::chrono::system_clock::now(), ""));

    init_locale();

    paramset::definitions defs = {
        {"resembla_measure", STR(weighted_word_edit_distance), {"resembla", "measure"}, "measure", 'm', "measure for scoring"},
        {"resembla_threshold", 0.2, {"resembla", "threshold"}, "threshold", 't', "measure for scoring"},
        {"resembla_max_response", 20, {"resembla", "max_response"}, "max-response", 'n', "max number of responses from Resembla"},
        {"resembla_max_reranking_num", 1000, {"resembla", "max_reranking_num"}, "max-reranking-num", 'r', "max number of reranking texts in Resembla"},
        {"simstring_ngram_unit", 2, {"simstring", "ngram_unit"}, "simstring-ngram-unit", 'N', "Unit of N-gram for SimString"},
        {"simstring_text_preprocess", "asis", {"simstring", "text_preprocess"}, "simstring-text-preprocess", 'P', "preprocessing method for texts to create index"},
        {"simstring_measure_str", "cosine", {"simstring", "measure"}, "simstring-measure", 's', "SimString measure"},
        {"simstring_threshold", 0.2, {"simstring", "threshold"}, "simstring-threshold", 'T', "SimString threshold"},
        {"ed_simstring_ngram_unit", -1, {"edit_distance", "simstring_ngram_unit"}, "ed-simstring-ngram-unit", 0, "Unit of N-gram for input text"},
        {"ed_simstring_threshold", -1, {"edit_distance", "simstring_threshold"}, "ed-simstring-threshold", 0, "SimString threshold for edit distance"},
        {"ed_max_reranking_num", -1, {"edit_distance", "max_reranking_num"}, "ed-max-reranking-num", 0, "max number of reranking texts for edit distance"},
        {"ed_ensemble_weight", 0.5, {"edit_distance", "ensemble_weight"}, "ed-ensemble-weight", 0, "weight coefficient for edit distance in ensemble mode"},
        {"wwed_simstring_ngram_unit", -1, {"weighted_word_edit_distance", "simstring_ngram_unit"}, "wwed-simstring-ngram-unit", 0, "Unit of N-gram for input text"},
        {"wwed_simstring_threshold", -1, {"weighted_word_edit_distance", "simstring_threshold"}, "wwed-simstring-threshold", 0, "SimString threshold for weighted word edit distance"},
        {"wwed_max_reranking_num", -1, {"weighted_word_edit_distance", "max_reranking_num"}, "wwed-max-reranking-num", 0, "max number of reranking texts for weighted word edit distance"},
        {"wwed_mecab_options", "", {"weighted_word_edit_distance", "mecab_options"}, "wwed-mecab-options", 0, "MeCab options for weighted word edit distance"},
        {"wwed_base_weight", 1L, {"weighted_word_edit_distance", "base_weight"}, "wwed-base-weight", 0, "base weight for weighted word edit distance"},
        {"wwed_delete_insert_ratio", 10L, {"weighted_word_edit_distance", "delete_insert_ratio"}, "wwed-del-ins-ratio", 0, "cost ratio of deletion and insertion for weighted word edit distance"},
        {"wwed_noun_coefficient", 10L, {"weighted_word_edit_distance", "noun_coefficient"}, "wwed-noun-coefficient", 0, "coefficient of nouns for weighted word edit distance"},
        {"wwed_verb_coefficient", 10L, {"weighted_word_edit_distance", "verb_coefficient"}, "wwed-verb-coefficient", 0, "coefficient of verbs for weighted word edit distance"},
        {"wwed_adj_coefficient", 5L, {"weighted_word_edit_distance", "adj_coefficient"}, "wwed-adj-coefficient", 0, "coefficient of adjectives for weighted word edit distance"},
        {"wwed_ensemble_weight", 0.5, {"weighted_word_edit_distance", "ensemble_weight"}, "wwed-ensemble-weight", 0, "weight coefficient for weighted word edit distance in ensemble mode"},
        {"wped_simstring_ngram_unit", -1, {"weighted_pronunciation_edit_distance", "simstring_ngram_unit"}, "wped-simstring-ngram-unit", 0, "Unit of N-gram for pronunciation of input text"},
        {"wped_simstring_threshold", -1, {"weighted_pronunciation_edit_distance", "simstring_threshold"}, "wped-simstring-threshold", 0, "SimString threshold for weighted pronunciation edit distance"},
        {"wped_max_reranking_num", -1, {"weighted_pronunciation_edit_distance", "max_reranking_num"}, "wped-max-reranking-num", 0, "max number of reranking texts for weighted pronunciation edit distance"},
        {"wped_mecab_options", "", {"weighted_pronunciation_edit_distance", "mecab_options"}, "wped-mecab-options", 0, "MeCab options for weighted pronunciation edit distance"},
        {"wped_mecab_feature_pos", 7, {"weighted_pronunciation_edit_distance", "mecab_feature_pos"}, "wped-mecab-feature-pos", 0, "Position of pronunciation in feature for weighted pronunciation edit distance"},
        {"wped_mecab_pronunciation_of_marks", "", {"weighted_pronunciation_edit_distance", "mecab_pronunciation_of_marks"}, "wped-mecab-pronunciation-of-marks", 0, "pronunciation in MeCab features when input is a mark"},
        {"wped_base_weight", 1L, {"weighted_pronunciation_edit_distance", "base_weight"}, "wped-base-weight", 0, "base weight for weighted pronunciation edit distance"},
        {"wped_delete_insert_ratio", 10L, {"weighted_pronunciation_edit_distance", "delete_insert_ratio"}, "wped-del-ins-ratio", 0, "cost ratio of deletion and insertion for weighted pronunciation edit distance"},
        {"wped_ensemble_weight", 0.5, {"weighted_pronunciation_edit_distance", "ensemble_weight"}, "wped-ensemble-weight", 0, "weight coefficient for weighted pronunciation edit distance in ensemble mode"},
        {"wred_simstring_ngram_unit", -1, {"weighted_romaji_edit_distance", "simstring_ngram_unit"}, "wred-simstring-ngram-unit", 0, "Unit of N-gram for romaji notation of input text"},
        {"wred_simstring_threshold", -1, {"weighted_romaji_edit_distance", "simstring_threshold"}, "wred-simstring-threshold", 0, "SimString threshold for weighted romaji edit distance"},
        {"wred_max_reranking_num", -1, {"weighted_romaji_edit_distance", "max_reranking_num"}, "wred-max-reranking-num", 0, "max number of reranking texts for weighted romaji edit distance"},
        {"wred_mecab_options", "", {"weighted_romaji_edit_distance", "mecab_options"}, "wred-mecab-options", 0, "MeCab options for weighted romaji edit distance"},
        {"wred_mecab_feature_pos", 7, {"weighted_romaji_edit_distance", "mecab_feature_pos"}, "wred-mecab-feature-pos", 0, "Position of pronunciation in feature for weighted romaji edit distance"},
        {"wred_mecab_pronunciation_of_marks", "", {"weighted_romaji_edit_distance", "mecab_pronunciation_of_marks"}, "wred-mecab-pronunciation-of-marks", 0, "pronunciation in MeCab features when input is a mark"},
        {"wred_base_weight", 1L, {"weighted_romaji_edit_distance", "base_weight"}, "wred-base-weight", 0, "base weight for weighted romaji edit distance"},
        {"wred_delete_insert_ratio", 10L, {"weighted_romaji_edit_distance", "delete_insert_ratio"}, "wred-del-ins-ratio", 0, "cost ratio of deletion and insertion for weighted romaji edit distance"},
        {"wred_uppercase_coefficient", 1L, {"weighted_romaji_edit_distance", "uppercase_coefficient"}, "wred-uppercase-coefficient", 0, "coefficient for uppercase letters for weighted romaji edit distance"},
        {"wred_lowercase_coefficient", 1L, {"weighted_romaji_edit_distance", "lowercase_coefficient"}, "wred-lowercase-coefficient", 0, "coefficient for lowercase letters for weighted romaji edit distance"},
        {"wred_vowel_coefficient", 1L, {"weighted_romaji_edit_distance", "vowel_coefficient"}, "wred-vowel-coefficient", 0, "coefficient for vowels for weighted romaji edit distance"},
        {"wred_consonant_coefficient", 1L, {"weighted_romaji_edit_distance", "consonant_coefficient"}, "wred-consonant-coefficient", 0, "coefficient for consonants for weighted romaji edit distance"},
        {"wred_case_mismatch_cost", 1L, {"weighted_romaji_edit_distance", "case_mismatch_cost"}, "wred-case-mismatch-cost", 0, "cost to replace case mismatches for weighted romaji edit distance"},
        {"wred_similar_letter_cost", 1L, {"weighted_romaji_edit_distance", "similar_letter_cost"}, "wred-similar-letter-cost", 0, "cost to replace similar letters for weighted romaji edit distance"},
        {"wred_mismatch_cost_path", "", {"weighted_romaji_edit_distance", "mismatch_cost_path"}, "wred-mismatch-cost-path", 0, "costs to replace similar letters for weighted romaji edit distance"},
        {"wred_ensemble_weight", 0.5, {"weighted_romaji_edit_distance", "ensemble_weight"}, "wred-ensemble-weight", 0, "weight coefficient for weighted romaji edit distance in ensemble mode"},
        {"km_simstring_ngram_unit", -1, {"keyword_match", "simstring_ngram_unit"}, "km-simstring-ngram-unit", 0, "Unit of N-gram for input text"},
        {"km_simstring_threshold", -1, {"keyword_match", "simstring_threshold"}, "km-simstring-threshold", 0, "SimString threshold for keyword match"},
        {"km_max_reranking_num", -1, {"keyword_match", "max_reranking_num"}, "km-max-reranking-num", 0, "max number of reranking texts for keyword match"},
        {"km_ensemble_weight", 0.2, {"keyword_match", "ensemble_weight"}, "km-ensemble-weight", 0, "weight coefficient for keyword match in ensemble mode"},
        {"svr_max_candidate", 2000, {"svr", "max_candidate"}, "svr-max-candidate", 0, "max number of candidates for support vector regression"},
        {"svr_features_path", "features.tsv", {"svr", "features_path"}, "svr-features-path", 0, "feature definition file for support vector regression"},
        {"svr_patterns_home", ".", {"svr", "patterns_home"}, "svr-patterns-home", 0, "directory for pattern files for regular expression-based feature extractors"},
        {"svr_model_path", "model", {"svr", "model_path"}, "svr-model-path", 0, "LibSVM model file"},
        {"svr_features_col", 2, {"svr", "features_col"}, "svr-features-col", 0, "column number of features for support vector regression"},
        {"corpus_path", "", {"common", "corpus_path"}},
        {"id_col", 0, {"common", "id_col"}, "id-col", 0, "column number (starts with 1) of ID in corpus rows. ignored if id_col==0"},
        {"text_col", 1, {"common", "text_col"}, "text-col", 0, "column mumber of text in corpus rows"},
        {"features_col", 2, {"common", "features_col"}, "features-col", 0, "column number of features in corpus rows"},
        {"conf_path", "", "config", 'c', "config file path"}
    };
    paramset::manager pm(defs);
    try{
        pm.load(argc, argv, "config");

        std::string corpus_path = read_value_with_rest(pm, "corpus_path", ""); // must not be empty
        int resembla_max_response = pm.get<int>("resembla_max_response");
        double resembla_threshold = pm.get<double>("resembla_threshold");

        int default_simstring_ngram_unit = pm["simstring_ngram_unit"];
        int ed_simstring_ngram_unit = pm.get<int>("ed_simstring_ngram_unit") != -1 ?
            pm.get<int>("ed_simstring_ngram_unit") : default_simstring_ngram_unit;
        int wwed_simstring_ngram_unit = pm.get<int>("wwed_simstring_ngram_unit") != -1 ?
            pm.get<int>("wwed_simstring_ngram_unit") : default_simstring_ngram_unit;
        int wped_simstring_ngram_unit = pm.get<int>("wped_simstring_ngram_unit") != -1 ?
            pm.get<int>("wped_simstring_ngram_unit") : default_simstring_ngram_unit;
        int wred_simstring_ngram_unit = pm.get<int>("wred_simstring_ngram_unit") != -1 ?
            pm.get<int>("wred_simstring_ngram_unit") : default_simstring_ngram_unit;
        auto resembla_measures = split_to_resembla_measures(pm["resembla_measure"]);

        double default_simstring_threshold = pm["simstring_threshold"];
        double ed_simstring_threshold = pm.get<double>("ed_simstring_threshold") != -1 ?
            pm.get<double>("ed_simstring_threshold") : default_simstring_threshold;
        double wwed_simstring_threshold = pm.get<double>("wwed_simstring_threshold") != -1 ?
            pm.get<double>("wwed_simstring_threshold") : default_simstring_threshold;
        double wped_simstring_threshold = pm.get<double>("wped_simstring_threshold") != -1 ?
            pm.get<double>("wped_simstring_threshold") : default_simstring_threshold;
        double wred_simstring_threshold = pm.get<double>("wred_simstring_threshold") != -1 ?
            pm.get<double>("wred_simstring_threshold") : default_simstring_threshold;

        int default_max_reranking_num = pm["resembla_max_reranking_num"];
        int ed_max_reranking_num = pm.get<int>("ed_max_reranking_num") != -1 ?
            pm.get<int>("ed_max_reranking_num") : default_max_reranking_num;
        int wwed_max_reranking_num = pm.get<int>("wwed_max_reranking_num") != -1 ?
            pm.get<int>("wwed_max_reranking_num") : default_max_reranking_num;
        int wped_max_reranking_num = pm.get<int>("wped_max_reranking_num") != -1 ?
            pm.get<int>("wped_max_reranking_num") : default_max_reranking_num;
        int wred_max_reranking_num = pm.get<int>("wred_max_reranking_num") != -1 ?
            pm.get<int>("wred_max_reranking_num") : default_max_reranking_num;

        std::cerr << "Configurations:" << std::endl;
        std::cerr << "  Common:" << std::endl;
        std::cerr << "    corpus_path=" << corpus_path << std::endl;
        std::cerr << "  SimString:" << std::endl;
        std::cerr << "    ngram_unit=" << default_simstring_ngram_unit << std::endl;
        std::cerr << "    measure=" << pm.get<std::string>("simstring_measure_str") << std::endl;
        std::cerr << "    threshold=" << default_simstring_threshold << std::endl;
        std::cerr << "  Resembla:" << std::endl;
        std::cerr << "    measure=" << pm.get<std::string>("resembla_measure") << std::endl;
        std::cerr << "    threshold=" << pm.get<double>("resembla_threshold") << std::endl;
        std::cerr << "    max_reranking_num=" << default_max_reranking_num << std::endl;
        std::cerr << "    max_response=" << pm.get<int>("resembla_max_response") << std::endl;
        for(const auto& resembla_measure: resembla_measures){
            if(resembla_measure == edit_distance && pm.get<double>("ed_ensemble_weight") > 0){
                std::cerr << "  Edit distance:" << std::endl;
                std::cerr << "    simstring_ngram_unit=" << ed_simstring_ngram_unit << std::endl;
                std::cerr << "    simstring_threshold=" << ed_simstring_threshold << std::endl;
                std::cerr << "    max_reranking_num=" << ed_max_reranking_num << std::endl;
                std::cerr << "    ensemble_weight=" << pm.get<double>("ed_ensemble_weight") << std::endl;
            }
            if(resembla_measure == weighted_word_edit_distance && pm.get<double>("wwed_ensemble_weight") > 0){
                std::cerr << "  Weighted word edit distance:" << std::endl;
                std::cerr << "    simstring_ngram_unit=" << wwed_simstring_ngram_unit << std::endl;
                std::cerr << "    simstring_threshold=" << wwed_simstring_threshold << std::endl;
                std::cerr << "    max_reranking_num=" << wwed_max_reranking_num << std::endl;
                std::cerr << "    mecab_options=" << pm.get<std::string>("wwed_mecab_options") << std::endl;
                std::cerr << "    base_weight=" << pm.get<double>("wwed_base_weight") << std::endl;
                std::cerr << "    delete_insert_ratio=" << pm.get<double>("wwed_delete_insert_ratio") << std::endl;
                std::cerr << "    noun_coefficient=" << pm.get<double>("wwed_noun_coefficient") << std::endl;
                std::cerr << "    verb_coefficient=" << pm.get<double>("wwed_verb_coefficient") << std::endl;
                std::cerr << "    adj_coefficient=" << pm.get<double>("wwed_adj_coefficient") << std::endl;
                std::cerr << "    ensemble_weight=" << pm.get<double>("wwed_ensemble_weight") << std::endl;
            }
            else if(resembla_measure == weighted_pronunciation_edit_distance && pm.get<double>("wped_ensemble_weight") > 0){
                std::cerr << "  Weighted pronunciation edit distance:" << std::endl;
                std::cerr << "    simstring_ngram_unit=" << wped_simstring_ngram_unit << std::endl;
                std::cerr << "    simstring_threshold=" << wped_simstring_threshold << std::endl;
                std::cerr << "    max_reranking_num=" << wped_max_reranking_num << std::endl;
                std::cerr << "    mecab_options=" << pm.get<std::string>("wped_mecab_options") << std::endl;
                std::cerr << "    mecab_feature_pos=" << pm.get<int>("wped_mecab_feature_pos") << std::endl;
                std::cerr << "    mecab_pronunciation_of_marks=" << pm.get<std::string>("wped_mecab_pronunciation_of_marks") << std::endl;
                std::cerr << "    base_weight=" << pm.get<double>("wped_base_weight") << std::endl;
                std::cerr << "    delete_insert_ratio=" << pm.get<double>("wped_delete_insert_ratio") << std::endl;
                std::cerr << "    ensemble_weight=" << pm.get<double>("wped_ensemble_weight") << std::endl;
            }
            else if(resembla_measure == weighted_romaji_edit_distance && pm.get<double>("wred_ensemble_weight") > 0){
                std::cerr << "  Weighted romaji edit distance:" << std::endl;
                std::cerr << "    simstring_ngram_unit=" << wred_simstring_ngram_unit << std::endl;
                std::cerr << "    simstring_threshold=" << wred_simstring_threshold << std::endl;
                std::cerr << "    max_reranking_num=" << wred_max_reranking_num << std::endl;
                std::cerr << "    mecab_options=" << pm.get<std::string>("wred_mecab_options") << std::endl;
                std::cerr << "    mecab_feature_pos=" << pm.get<int>("wred_mecab_feature_pos") << std::endl;
                std::cerr << "    mecab_pronunciation_of_marks=" << pm.get<std::string>("wred_mecab_pronunciation_of_marks") << std::endl;
                std::cerr << "    base_weight=" << pm.get<double>("wred_base_weight") << std::endl;
                std::cerr << "    delete_insert_ratio=" << pm.get<double>("wred_delete_insert_ratio") << std::endl;
                std::cerr << "    uppercase_coefficient=" << pm.get<double>("wred_uppercase_coefficient") << std::endl;
                std::cerr << "    lowercase_coefficient=" << pm.get<double>("wred_lowercase_coefficient") << std::endl;
                std::cerr << "    vowel_coefficient=" << pm.get<double>("wred_vowel_coefficient") << std::endl;
                std::cerr << "    consonant_coefficient=" << pm.get<double>("wred_consonant_coefficient") << std::endl;
                std::cerr << "    case_mismatch_cost=" << pm.get<double>("wred_case_mismatch_cost") << std::endl;
                std::cerr << "    similar_letter_cost=" << pm.get<double>("wred_similar_letter_cost") << std::endl;
                std::cerr << "    ensemble_weight=" << pm.get<double>("wred_ensemble_weight") << std::endl;
            }
        }
        time_points.push_back(std::make_pair(std::chrono::system_clock::now(), "config"));

        // load test data and create index for each measure
        TestData test_data;
        for(const auto& resembla_measure: resembla_measures){
            std::string db_path = db_path_from_resembla_measure(corpus_path, resembla_measure);
            std::string inverse_path = inverse_path_from_resembla_measure(corpus_path, resembla_measure);
            switch(resembla_measure){
                case svr: {
                    auto features = load_features(pm.get<std::string>("svr_features_path"));
                    if(features.empty()){
                        throw std::runtime_error("no feature");
                    }
                    const auto& base_feature = features[0][0];

                    FeatureExtractor extractor;
                    for(const auto& feature: features){
                        const auto& name = feature[0];
                        if(name == base_feature){
                            continue;
                        }

                        const auto& feature_extractor_type = feature[1];
                        if(feature_extractor_type == "re"){
                            extractor.append(name, std::make_shared<RegexFeatureExtractor>(pm.get<std::string>("svr_patterns_home") + "/" + name + ".tsv"));
                        }
                        else if(feature_extractor_type == "date_period"){
                            extractor.append(name, std::make_shared<DatePeriodFeatureExtractor>());
                        }
                        else if(feature_extractor_type == "time_period"){
                            extractor.append(name, std::make_shared<TimePeriodFeatureExtractor>());
                        }
                        else if(feature_extractor_type != "-"){
                            throw std::runtime_error("unknown feature extractor type: " + feature_extractor_type);
                        }
                    }
                    test_data = prepare_data(corpus_path, db_path, inverse_path, ed_simstring_ngram_unit, extractor);
                    break;
                }
                case edit_distance: {
                    if(pm.get<double>("ed_ensemble_weight") > 0){
                        AsIsSequenceBuilder<std::wstring> builder;
                        test_data = prepare_data(corpus_path, db_path, inverse_path, ed_simstring_ngram_unit, builder);
                    }
                    break;
                }
                case weighted_word_edit_distance: {
                    if(pm.get<double>("wwed_ensemble_weight") > 0){
                        WeightedSequenceBuilder<WordSequenceBuilder, FeatureMatchWeight> builder(
                            WordSequenceBuilder(pm.get<std::string>("wwed_mecab_options")),
                            FeatureMatchWeight(pm.get<double>("wwed_base_weight"),
                                pm.get<double>("wwed_delete_insert_ratio"), pm.get<double>("wwed_noun_coefficient"),
                                pm.get<double>("wwed_verb_coefficient"), pm.get<double>("wwed_adj_coefficient")));
                        test_data = prepare_data(corpus_path, db_path, inverse_path, wwed_simstring_ngram_unit, builder);
                    }
                    break;
                }
                case weighted_pronunciation_edit_distance: {
                    if(pm.get<double>("wped_ensemble_weight") > 0){
                        PronunciationSequenceBuilder builder(pm.get<std::string>("wped_mecab_options"),
                                pm.get<int>("wped_mecab_feature_pos"), pm.get<std::string>("wped_mecab_pronunciation_of_marks"));
                        test_data = prepare_data(corpus_path, db_path, inverse_path, wped_simstring_ngram_unit, builder);
                    }
                    break;
                }
                case weighted_romaji_edit_distance: {
                    if(pm.get<double>("wred_ensemble_weight") > 0){
                        WeightedSequenceBuilder<RomajiSequenceBuilder, RomajiWeight> builder(
                            RomajiSequenceBuilder(pm.get<std::string>("wred_mecab_options"),
                                pm.get<int>("wred_mecab_feature_pos"), pm.get<std::string>("wred_mecab_pronunciation_of_marks")),
                            RomajiWeight(pm.get<double>("wred_base_weight"), pm.get<double>("wred_delete_insert_ratio"),
                                pm.get<double>("wred_uppercase_coefficient"), pm.get<double>("wred_lowercase_coefficient"),
                                pm.get<double>("wred_vowel_coefficient"), pm.get<double>("wred_consonant_coefficient")));
                        test_data = prepare_data(corpus_path, db_path, inverse_path, wred_simstring_ngram_unit, builder);
                    }
                    break;
                }
                case keyword_match: {
                    if(pm.get<double>("km_ensemble_weight") > 0){
                        KeywordMatchPreprocessor<string_type> builder;
                        test_data = prepare_data(corpus_path, db_path, inverse_path, wred_simstring_ngram_unit, builder);
                    }
                    break;
                }
                default:
                    break;
            }
        }
        if(test_data.empty()){
            throw std::invalid_argument("no data for evaluation");
        }
        time_points.push_back(std::make_pair(std::chrono::system_clock::now(), "index"));

        // initialize Resembla with created indexes
        auto resembla = construct_resembla(corpus_path, pm);
        time_points.push_back(std::make_pair(std::chrono::system_clock::now(), "load"));

        // execute evaluation
        std::vector<std::vector<ResemblaInterface::output_type>> answers;
        for(const auto& d: test_data){
            for(const auto& i: d.second){
                answers.push_back(resembla->find(i.first, resembla_threshold, resembla_max_response));
            }
        }
        time_points.push_back(std::make_pair(std::chrono::system_clock::now(), "answer"));

        // output results
        auto it = std::begin(answers);
        std::wcout <<
            "freq" << DELIMITER <<
            "input" << DELIMITER <<
            "pred" << DELIMITER <<
            "true" << DELIMITER <<
            "score" << DELIMITER <<
            "score_of_correct_answer" << DELIMITER <<
            "rank_of_correct_answer" << std::endl;
        for(const auto& d: test_data){
            const auto& original = d.first;
            for(const auto& i: d.second){
                const auto& query = i.first;
                const auto& freq = i.second;

                auto response = *it++;

                std::wstring best = !response.empty() ? response[0].text : L"NONE";
                double score_best = !response.empty() ? response[0].score : -1;
                auto p = std::find_if(response.begin(), response.end(),
                        [original](ResemblaInterface::output_type& r) -> bool {return original == r.text;});
                int rank_correct = p != response.end() ? p - response.begin() + 1 : -1;
                double score_correct = rank_correct != -1 ? response[rank_correct - 1].score : -1;

                std::wcout <<
                    freq << DELIMITER <<
                    query << DELIMITER <<
                    best << DELIMITER <<
                    original << DELIMITER <<
                    score_best << DELIMITER <<
                    score_correct << DELIMITER <<
                    rank_correct << std::endl;
            }
        }
        time_points.push_back(std::make_pair(std::chrono::system_clock::now(), "output"));

        std::cerr << std::endl << "computation time" << std::endl;
        for(size_t i = 1; i < time_points.size(); ++i){
            auto t = std::chrono::duration_cast<std::chrono::microseconds>(time_points[i].first - time_points[i - 1].first).count();
            std::cerr << time_points[i].second << "\t" << (t / 1000000.0) << std::endl;
        }
    }
    catch(const std::exception& e){
        std::cerr << "error: " << e.what() << std::endl;
        exit(1);
    }

    return 0;
}
