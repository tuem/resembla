/*
Resembla
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

#include <simstring/simstring.h>
#include <paramset.hpp>

#include "resembla_util.hpp"
#include "csv_reader.hpp"

#include "measure/asis_preprocessor.hpp"
#include "measure/pronunciation_preprocessor.hpp"
#include "measure/romaji_preprocessor.hpp"

#include "history.hpp"

using namespace resembla;

// list of {true_text, list of {input_text, freq}}
using TestData = std::vector<std::pair<string_type, std::vector<std::pair<string_type, int>>>>;

constexpr auto DELIMITER = column_delimiter<string_type::value_type>();
constexpr char WORD_FREQ_SEPARATOR = '/';
constexpr char NONE[] = "NONE";

// generates SimString index and test data
template<typename Indexer>
TestData prepare_data(std::string test_data_path, std::string db_path, std::string inverse_path,
        int simstring_ngram_unit, std::shared_ptr<Indexer> index)
{
    std::unordered_map<string_type, std::unordered_set<string_type>> inverse;
    TestData test_data;

    simstring::ngram_generator gen(simstring_ngram_unit, false);
    simstring::writer_base<string_type> dbw(gen, db_path);
    bool first = true;
    for(const auto& columns: CsvReader<>(test_data_path, 1, DELIMITER)){
        // skip header
        if(first){
            first = false;
            continue;
        }

        // format: {true}\t{input0}/{freq0}\t{input1}\t{freq1}...
        const auto original = cast_string<string_type>(columns[0]);
        auto indexed = (*index)(original);
        if(inverse.count(indexed) == 0){
            dbw.insert(indexed);
            inverse[indexed] = {original};
        }
        else{
            inverse[indexed].insert(original);
        }
        test_data.push_back(std::make_pair(original, std::vector<std::pair<string_type, int>>()));

        for(size_t i = 1; i < columns.size(); ++i){
            auto values = split(columns[i], WORD_FREQ_SEPARATOR);
            auto text = cast_string<string_type>(values[0]);
            if(text.empty()){
                continue;
            }

            int freq = 0;
            if(values.size() > 1){
                freq = std::stoi(values[1]);
            }

            test_data.back().second.push_back(std::make_pair(text, freq));
        }
    }
    dbw.close();

    std::basic_ofstream<string_type::value_type> ofs;
    ofs.open(inverse_path);
    for(auto p: inverse){
        for(auto original: p.second){
            ofs << p.first << DELIMITER << original << std::endl;
        }
    }
    return test_data;
}

int main(int argc, char* argv[])
{
    History history;
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
        {"index_romaji_mecab_options", "", {"index", "romaji", "mecab_options"}, "index-romaji-mecab-options", 0, "MeCab options for romaji indexer"},
        {"index_romaji_mecab_feature_pos", 7, {"index", "romaji", "mecab_feature_pos"}, "index-romaji-mecab-feature-pos", 0, "Position of pronunciation in feature for romaji indexer"},
        {"index_romaji_mecab_pronunciation_of_marks", "", {"index", "romaji", "mecab_pronunciation_of_marks"}, "index-romaji-mecab-pronunciation-of-marks", 0, "pronunciation in MeCab features when input is a mark"},
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
        {"wped_letter_weight_path", "", {"weighted_pronunciation_edit_distance", "letter_weight_path"}, "wped-letter-weight-path", 0, "weights of kana letters for weighted pronunciation edit distance"},
        {"wped_mismatch_cost_path", "", {"weighted_pronunciation_edit_distance", "mismatch_cost_path"}, "wped-mismatch-cost-path", 0, "costs to replace similar letters for weighted pronunciation edit distance"},
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
        {"ensemble_simstring_ngram_unit", -1, {"ensemble", "simstring_ngram_unit"}, "ensemble-simstring-ngram-unit", 0, "Unit of N-gram for romaji notation of input text"},
        {"ensemble_simstring_threshold", -1, {"ensemble", "simstring_threshold"}, "ensemble-simstring-threshold", 0, "SimString threshold for ensemble"},
        {"ensemble_max_candidate", 100, {"ensemble", "max_candidate"}, "ensemble-max-candidate", 0, "max number of candidates for ensemble method"},
        {"svr_simstring_ngram_unit", -1, {"svr", "simstring_ngram_unit"}, "svr-simstring-ngram-unit", 0, "Unit of N-gram for input text"},
        {"svr_simstring_threshold", -1, {"svr", "simstring_threshold"}, "svr-simstring-threshold", 0, "SimString threshold for svr"},
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

        if(pm.rest.size() > 0){
            pm["corpus_path"] = pm.rest.front().as<std::string>();
        }
        if(pm.get<std::string>("corpus_path").empty()){
            throw std::invalid_argument("no corpus file specified");
        }
        std::string corpus_path = pm.get<std::string>("corpus_path");

        int resembla_max_response = pm.get<int>("resembla_max_response");
        double resembla_threshold = pm.get<double>("resembla_threshold");
        auto resembla_measures = split_to_resembla_measures(pm["resembla_measure"]);
        int ensemble_count = 0;
        for(auto resembla_measure: resembla_measures){
            switch(resembla_measure){
                case edit_distance:
                case weighted_word_edit_distance:
                case weighted_pronunciation_edit_distance:
                case weighted_romaji_edit_distance:
                case keyword_match:
                    ++ensemble_count;
                    break;
                default:
                    break;
            }
        }
        bool use_ensemble = ensemble_count > 1;

        pm["simstring_measure"] = simstring_measure_from_string(pm.get<std::string>("simstring_measure_str"));

        if(pm.get<int>("ed_simstring_ngram_unit") == -1){
            pm["ed_simstring_ngram_unit"] = pm.get<int>("simstring_ngram_unit");
        }
        if(pm.get<int>("wwed_simstring_ngram_unit") == -1){
            pm["wwed_simstring_ngram_unit"] = pm.get<int>("simstring_ngram_unit");
        }
        if(pm.get<int>("wped_simstring_ngram_unit") == -1){
            pm["wped_simstring_ngram_unit"] = pm.get<int>("simstring_ngram_unit");
        }
        if(pm.get<int>("wred_simstring_ngram_unit") == -1){
            pm["wred_simstring_ngram_unit"] = pm.get<int>("simstring_ngram_unit");
        }
        if(pm.get<int>("km_simstring_ngram_unit") == -1){
            pm["km_simstring_ngram_unit"] = pm.get<int>("simstring_ngram_unit");
        }
        if(pm.get<int>("ensemble_simstring_ngram_unit") == -1){
            pm["ensemble_simstring_ngram_unit"] = pm.get<int>("simstring_ngram_unit");
        }
        if(pm.get<int>("svr_simstring_ngram_unit") == -1){
            pm["svr_simstring_ngram_unit"] = pm.get<int>("simstring_ngram_unit");
        }

        if(pm.get<double>("ed_simstring_threshold") == -1){
            pm["ed_simstring_threshold"] = pm.get<double>("simstring_threshold");
        }
        if(pm.get<double>("wwed_simstring_threshold") == -1){
            pm["wwed_simstring_threshold"] = pm.get<double>("simstring_threshold");
        }
        if(pm.get<double>("wped_simstring_threshold") == -1){
            pm["wped_simstring_threshold"] = pm.get<double>("simstring_threshold");
        }
        if(pm.get<double>("wred_simstring_threshold") == -1){
            pm["wred_simstring_threshold"] = pm.get<double>("simstring_threshold");
        }
        if(pm.get<double>("km_simstring_threshold") == -1){
            pm["km_simstring_threshold"] = pm.get<double>("simstring_threshold");
        }
        if(pm.get<double>("ensemble_simstring_threshold") == -1){
            pm["ensemble_simstring_threshold"] = pm.get<double>("simstring_threshold");
        }
        if(pm.get<double>("svr_simstring_threshold") == -1){
            pm["svr_simstring_threshold"] = pm.get<double>("simstring_threshold");
        }

        if(pm.get<int>("ed_max_reranking_num") == -1){
            pm["ed_max_reranking_num"] = pm.get<int>("resembla_max_reranking_num");
        }
        if(pm.get<int>("wwed_max_reranking_num") == -1){
            pm["wwed_max_reranking_num"] = pm.get<int>("resembla_max_reranking_num");
        }
        if(pm.get<int>("wped_max_reranking_num") == -1){
            pm["wped_max_reranking_num"] = pm.get<int>("resembla_max_reranking_num");
        }
        if(pm.get<int>("wred_max_reranking_num") == -1){
            pm["wred_max_reranking_num"] = pm.get<int>("resembla_max_reranking_num");
        }
        if(pm.get<int>("km_max_reranking_num") == -1){
            pm["km_max_reranking_num"] = pm.get<int>("resembla_max_reranking_num");
        }

        std::cerr << "  Common:" << std::endl;
        std::cerr << "    corpus_path=" << corpus_path << std::endl;
        std::cerr << "  SimString:" << std::endl;
        std::cerr << "    ngram_unit=" << pm.get<int>("simstring_ngram_unit") << std::endl;
        std::cerr << "    measure=" << pm.get<std::string>("simstring_measure_str") << std::endl;
        std::cerr << "    threshold=" << pm.get<double>("simstring_threshold") << std::endl;
        std::cerr << "  Resembla:" << std::endl;
        std::cerr << "    measure=" << pm.get<std::string>("resembla_measure") << std::endl;
        std::cerr << "    threshold=" << pm.get<double>("resembla_threshold") << std::endl;
        std::cerr << "    max_reranking_num=" << pm.get<int>("resembla_max_reranking_num") << std::endl;
        std::cerr << "    max_response=" << pm.get<int>("resembla_max_response") << std::endl;
        if(use_ensemble){
            std::cerr << "  measure=" << STR(ensemble) << std::endl;
            std::cerr << "    simstring_ngram_unit=" << pm.get<int>("ensemble_simstring_ngram_unit") << std::endl;
            std::cerr << "    simstring_threshold=" << pm.get<int>("ensemble_simstring_threshold") << std::endl;
            std::cerr << "    max_candidate=" << pm.get<int>("ensemble_max_candidate") << std::endl;
        }
        for(const auto& resembla_measure: resembla_measures){
            if(resembla_measure == edit_distance && pm.get<double>("ed_ensemble_weight") > 0){
                std::cerr << "  Edit distance:" << std::endl;
                std::cerr << "    simstring_ngram_unit=" << pm.get<int>("ed_simstring_ngram_unit") << std::endl;
                std::cerr << "    simstring_threshold=" << pm.get<double>("ed_simstring_threshold") << std::endl;
                std::cerr << "    max_reranking_num=" << pm.get<int>("ed_max_reranking_num") << std::endl;
                std::cerr << "    ensemble_weight=" << pm.get<double>("ed_ensemble_weight") << std::endl;
            }
            if(resembla_measure == weighted_word_edit_distance && pm.get<double>("wwed_ensemble_weight") > 0){
                std::cerr << "  Weighted word edit distance:" << std::endl;
                std::cerr << "    simstring_ngram_unit=" << pm.get<int>("wwed_simstring_ngram_unit") << std::endl;
                std::cerr << "    simstring_threshold=" << pm.get<double>("wwed_simstring_threshold") << std::endl;
                std::cerr << "    max_reranking_num=" << pm.get<int>("wwed_max_reranking_num") << std::endl;
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
                std::cerr << "    simstring_ngram_unit=" << pm.get<int>("wped_simstring_ngram_unit") << std::endl;
                std::cerr << "    simstring_threshold=" << pm.get<double>("wped_simstring_threshold") << std::endl;
                std::cerr << "    max_reranking_num=" << pm.get<int>("wped_max_reranking_num") << std::endl;
                std::cerr << "    mecab_options=" << pm.get<std::string>("wped_mecab_options") << std::endl;
                std::cerr << "    mecab_feature_pos=" << pm.get<int>("wped_mecab_feature_pos") << std::endl;
                std::cerr << "    mecab_pronunciation_of_marks=" << pm.get<std::string>("wped_mecab_pronunciation_of_marks") << std::endl;
                std::cerr << "    base_weight=" << pm.get<double>("wped_base_weight") << std::endl;
                std::cerr << "    delete_insert_ratio=" << pm.get<double>("wped_delete_insert_ratio") << std::endl;
                std::cerr << "    letter_weight_path=" << pm.get<std::string>("wped_letter_weight_path") << std::endl;
                std::cerr << "    mismatch_cost_path=" << pm.get<std::string>("wped_mismatch_cost_path") << std::endl;
                std::cerr << "    ensemble_weight=" << pm.get<double>("wped_ensemble_weight") << std::endl;
            }
            else if(resembla_measure == weighted_romaji_edit_distance && pm.get<double>("wred_ensemble_weight") > 0){
                std::cerr << "  Weighted romaji edit distance:" << std::endl;
                std::cerr << "    simstring_ngram_unit=" << pm.get<int>("wred_simstring_ngram_unit") << std::endl;
                std::cerr << "    simstring_threshold=" << pm.get<double>("wred_simstring_threshold") << std::endl;
                std::cerr << "    max_reranking_num=" << pm.get<int>("wred_max_reranking_num") << std::endl;
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
                std::cerr << "    mismatch_cost_path=" << pm.get<std::string>("wred_mismatch_cost_path") << std::endl;
                std::cerr << "    ensemble_weight=" << pm.get<double>("wred_ensemble_weight") << std::endl;
            }
            else if(resembla_measure == keyword_match && pm.get<double>("km_ensemble_weight") > 0){
                std::cerr << "  Keyword matching:" << std::endl;
                std::cerr << "    simstring_ngram_unit=" << pm.get<int>("km_simstring_ngram_unit") << std::endl;
                std::cerr << "    simstring_threshold=" << pm.get<double>("km_simstring_threshold") << std::endl;
                std::cerr << "    max_reranking_num=" << pm.get<int>("km_max_reranking_num") << std::endl;
            }
            else if(resembla_measure == svr){
                std::cerr << "  SVR:" << std::endl;
                std::cerr << "    simstring_ngram_unit=" << pm.get<int>("svr_simstring_ngram_unit") << std::endl;
                std::cerr << "    simstring_threshold=" << pm.get<double>("svr_simstring_threshold") << std::endl;
                std::cerr << "    max_candidate=" << pm.get<int>("svr_max_candidate") << std::endl;
                std::cerr << "    features_path=" << pm.get<std::string>("svr_features_path") << std::endl;
                std::cerr << "    patterns_home=" << pm.get<std::string>("svr_patterns_home") << std::endl;
                std::cerr << "    model_path=" << pm.get<std::string>("svr_model_path") << std::endl;
            }
        }
        history.record("config");

        // load test data and create index for each measure
        TestData test_data;
        for(const auto& resembla_measure: resembla_measures){
            std::string db_path = db_path_from_resembla_measure(corpus_path, resembla_measure);
            std::string inverse_path = inverse_path_from_resembla_measure(corpus_path, resembla_measure);
            switch(resembla_measure){
                case svr: {
                    auto indexer = std::make_shared<RomajiPreprocessor>(pm.get<std::string>("index_romaji_mecab_options"),
                            pm.get<int>("index_romaji_mecab_feature_pos"), pm.get<std::string>("index_romaji_mecab_pronunciation_of_marks"));
                    test_data = prepare_data(corpus_path, db_path, inverse_path, pm.get<int>("svr_simstring_ngram_unit"), indexer);

                    break;
                }
                case edit_distance: {
                    if(pm.get<double>("ed_ensemble_weight") > 0){
                        auto indexer = std::make_shared<AsIsPreprocessor<string_type>>();
                        test_data = prepare_data(corpus_path, db_path, inverse_path, pm.get<int>("ed_simstring_ngram_unit"), indexer);
                    }
                    break;
                }
                case weighted_word_edit_distance: {
                    if(pm.get<double>("wwed_ensemble_weight") > 0){
                        auto indexer = std::make_shared<AsIsPreprocessor<string_type>>();
                        test_data = prepare_data(corpus_path, db_path, inverse_path, pm.get<int>("wwed_simstring_ngram_unit"), indexer);
                    }
                    break;
                }
                case weighted_pronunciation_edit_distance: {
                    if(pm.get<double>("wped_ensemble_weight") > 0){
                        auto indexer = std::make_shared<PronunciationPreprocessor>(pm.get<std::string>("wped_mecab_options"),
                            pm.get<int>("wped_mecab_feature_pos"), pm.get<std::string>("wped_mecab_pronunciation_of_marks"));
                        test_data = prepare_data(corpus_path, db_path, inverse_path, pm.get<int>("wped_simstring_ngram_unit"), indexer);
                    }
                    break;
                }
                case weighted_romaji_edit_distance: {
                    if(pm.get<double>("wred_ensemble_weight") > 0){
                        auto indexer = std::make_shared<RomajiPreprocessor>(pm.get<std::string>("wred_mecab_options"),
                            pm.get<int>("wred_mecab_feature_pos"), pm.get<std::string>("wred_mecab_pronunciation_of_marks"));
                        test_data = prepare_data(corpus_path, db_path, inverse_path, pm.get<int>("wred_simstring_ngram_unit"), indexer);
                    }
                    break;
                }
                case keyword_match: {
                    if(pm.get<double>("km_ensemble_weight") > 0){
                        auto indexer = std::make_shared<AsIsPreprocessor<string_type>>();
                        test_data = prepare_data(corpus_path, db_path, inverse_path, pm.get<int>("km_simstring_ngram_unit"), indexer);
                    }
                    break;
                }
                default:
                    break;
            }
        }

        if(use_ensemble){
            std::string db_path = db_path_from_resembla_measure(corpus_path, ensemble);
            std::string inverse_path = inverse_path_from_resembla_measure(corpus_path, ensemble);

            auto indexer = std::make_shared<RomajiPreprocessor>(pm.get<std::string>("index_romaji_mecab_options"),
                pm.get<int>("index_romaji_mecab_feature_pos"),
                pm.get<std::string>("index_romaji_mecab_pronunciation_of_marks"));
            test_data = prepare_data(corpus_path, db_path, inverse_path, pm.get<int>("ensemble_simstring_ngram_unit"), indexer);
        }

        if(test_data.empty()){
            throw std::invalid_argument("no data for evaluation");
        }
        history.record("index");

        // initialize Resembla with created indexes
        auto resembla = construct_resembla(pm);
        history.record("load");

        // execute evaluation
        std::vector<std::vector<ResemblaInterface::output_type>> answers;
        for(const auto& d: test_data){
            for(const auto& i: d.second){
                answers.push_back(resembla->find(i.first, resembla_threshold, resembla_max_response));
            }
        }
        history.record("answer");

        // output results
        auto it = std::begin(answers);
        std::cout <<
            "freq" << "\t"<<
            "input" << "\t" <<
            "pred" << "\t" <<
            "true" << "\t" <<
            "score" << "\t" <<
            "score_of_correct_answer" << "\t" <<
            "rank_of_correct_answer" << std::endl;
        for(const auto& d: test_data){
            const auto& original = d.first;
            for(const auto& i: d.second){
                const auto query = cast_string<std::string>(i.first);
                const auto& freq = i.second;

                auto response = *it++;

                auto best = !response.empty() ? cast_string<std::string>(response[0].text) : NONE;
                double score_best = !response.empty() ? response[0].score : -1;
                auto p = std::find_if(std::begin(response), std::end(response),
                        [original](ResemblaInterface::output_type& r) -> bool {return original == r.text;});
                int rank_correct = p != std::end(response) ? p - std::begin(response) + 1 : -1;
                double score_correct = rank_correct != -1 ? response[rank_correct - 1].score : -1;

                std::cout <<
                    freq << "\t" <<
                    query << "\t" <<
                    best << "\t" <<
                    cast_string<std::string>(original) << "\t" <<
                    score_best << "\t" <<
                    score_correct << "\t" <<
                    rank_correct << std::endl;
            }
        }
        history.record("output");

        history.dump(std::cerr);
    }
    catch(const std::exception& e){
        std::cerr << "error: " << e.what() << std::endl;
        exit(1);
    }

    return 0;
}
