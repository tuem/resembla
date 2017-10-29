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
#include <unordered_map>
#include <set>
#include <memory>
#include <stdexcept>

#include <simstring/simstring.h>
#include <json.hpp>
#include <paramset.hpp>

#include "string_normalizer.hpp"
#include "resembla_util.hpp"

#include "measure/asis_preprocessor.hpp"
#include "measure/word_preprocessor.hpp"
#include "measure/pronunciation_preprocessor.hpp"
#include "measure/romaji_preprocessor.hpp"
#include "measure/weighted_sequence_builder.hpp"
#include "measure/keyword_match_preprocessor.hpp"

#include "measure/word_weight.hpp"
#include "measure/letter_weight.hpp"
#include "measure/romaji_weight.hpp"

#include "regression/extractor/feature_extractor.hpp"
#include "regression/extractor/regex_feature_extractor.hpp"
#include "regression/extractor/date_period_feature_extractor.hpp"
#include "regression/extractor/time_period_feature_extractor.hpp"

#include "measure/weighted_sequence_serializer.hpp"

using namespace resembla;

template<typename Indexer, typename Preprocessor>
void create_index(const std::string& corpus_path, const std::string& db_path, const std::string& index_path,
        int n, std::shared_ptr<Indexer> index_func, std::shared_ptr<Preprocessor> preprocess,
        size_t text_col, size_t features_col, std::shared_ptr<StringNormalizer> normalize)
{
    constexpr auto delimiter = column_delimiter<string_type::value_type>();
    std::unordered_map<string_type, std::set<string_type>> inserted;

    simstring::ngram_generator gen(n, false);
    simstring::writer_base<string_type> dbw(gen, db_path);
    for(const auto& columns: CsvReader<string_type>(corpus_path, text_col, delimiter)){
        auto original = columns[text_col - 1];
        const auto& normalized = normalize != nullptr ? (*normalize)(original) : original;
        const auto& indexed = (*index_func)(normalized);

        if(features_col > 0 && features_col - 1 < columns.size()){
            original += delimiter + columns[features_col - 1];
        }

        if(inserted.count(indexed) == 0){
            dbw.insert(indexed);
            inserted[indexed] = {original};
        }
        else{
            inserted[indexed].insert(original);
        }
    }
    dbw.close();

    std::basic_ofstream<string_type::value_type> ofs;
    ofs.open(index_path);
    for(auto p: inserted){
        for(auto original: p.second){
            auto columns = split(original, delimiter);
            auto normalized = normalize != nullptr ? (*normalize)(columns[0]) : columns[0];
            if(columns.size() > 1){
                normalized += delimiter + columns[1];
            }

            if(preprocess == nullptr){
                ofs << p.first << delimiter << columns[0] << std::endl;
            }
            else{
                nlohmann::json j = (*preprocess)(normalized, true);
                auto preprocessed = cast_string<string_type>(j.dump());
                ofs << p.first << delimiter << columns[0] << delimiter << preprocessed << std::endl;
            }
        }
    }
}

int main(int argc, char* argv[])
{
    init_locale();

    paramset::definitions defs = {
        {"simstring_ngram_unit", 2, {"simstring", "ngram_unit"}, "simstring-ngram-unit", 'N', "Unit of N-gram for SimString"},
        {"index_romaji_mecab_options", "", {"index", "romaji", "mecab_options"}, "index-romaji-mecab-options", 0, "MeCab options for romaji indexer"},
        {"index_romaji_mecab_feature_pos", 7, {"index", "romaji", "mecab_feature_pos"}, "index-romaji-mecab-feature-pos", 0, "Position of pronunciation in feature for romaji indexer"},
        {"index_romaji_mecab_pronunciation_of_marks", "", {"index", "romaji", "mecab_pronunciation_of_marks"}, "index-romaji-mecab-pronunciation-of-marks", 0, "pronunciation in MeCab features when input is a mark"},
        {"resembla_measure", STR(weighted_word_edit_distance), {"resembla", "measure"}, "measure", 'm', "measure for scoring"},
        {"ed_simstring_ngram_unit", -1, {"edit_distance", "simstring_ngram_unit"}, "ed-simstring-ngram-unit", 0, "Unit of N-gram for input text"},
        {"wwed_simstring_ngram_unit", -1, {"weighted_word_edit_distance", "simstring_ngram_unit"}, "wwed-simstring-ngram-unit", 0, "Unit of N-gram for input text"},
        {"wwed_mecab_options", "", {"weighted_word_edit_distance", "mecab_options"}, "wwed-mecab-options", 0, "MeCab options for weighted word edit distance"},
        {"wwed_base_weight", 1L, {"weighted_word_edit_distance", "base_weight"}, "wwed-base-weight", 0, "base weight for weighted word edit distance"},
        {"wwed_delete_insert_ratio", 10L, {"weighted_word_edit_distance", "delete_insert_ratio"}, "wwed-del-ins-ratio", 0, "cost ratio of deletion and insertion for weighted word edit distance"},
        {"wwed_noun_coefficient", 10L, {"weighted_word_edit_distance", "noun_coefficient"}, "wwed-noun-coefficient", 0, "coefficient of nouns for weighted word edit distance"},
        {"wwed_verb_coefficient", 10L, {"weighted_word_edit_distance", "verb_coefficient"}, "wwed-verb-coefficient", 0, "coefficient of verbs for weighted word edit distance"},
        {"wwed_adj_coefficient", 5L, {"weighted_word_edit_distance", "adj_coefficient"}, "wwed-adj-coefficient", 0, "coefficient of adjectives for weighted word edit distance"},
        {"wped_simstring_ngram_unit", -1, {"weighted_pronunciation_edit_distance", "simstring_ngram_unit"}, "wped-simstring-ngram-unit", 0, "Unit of N-gram for pronunciation of input text"},
        {"wped_mecab_options", "", {"weighted_pronunciation_edit_distance", "mecab_options"}, "wped-mecab-options", 0, "MeCab options for weighted pronunciation edit distance"},
        {"wped_mecab_feature_pos", 7, {"weighted_pronunciation_edit_distance", "mecab_feature_pos"}, "wped-mecab-feature-pos", 0, "Position of pronunciation in feature for weighted pronunciation edit distance"},
        {"wped_mecab_pronunciation_of_marks", "", {"weighted_pronunciation_edit_distance", "mecab_pronunciation_of_marks"}, "wped-mecab-pronunciation-of-marks", 0, "pronunciation in MeCab features when input is a mark"},
        {"wped_base_weight", 1L, {"weighted_pronunciation_edit_distance", "base_weight"}, "wped-base-weight", 0, "base weight for weighted pronunciation edit distance"},
        {"wped_delete_insert_ratio", 10L, {"weighted_pronunciation_edit_distance", "delete_insert_ratio"}, "wped-del-ins-ratio", 0, "cost ratio of deletion and insertion for weighted pronunciation edit distance"},
        {"wped_letter_weight_path", "", {"weighted_pronunciation_edit_distance", "letter_weight_path"}, "wped-letter-weight-path", 0, "weights of kana letters for weighted pronunciation edit distance"},
        {"wred_simstring_ngram_unit", -1, {"weighted_romaji_edit_distance", "simstring_ngram_unit"}, "wred-simstring-ngram-unit", 0, "Unit of N-gram for romaji notation of input text"},
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
        {"km_simstring_ngram_unit", -1, {"keyword_match", "simstring_ngram_unit"}, "km-simstring-ngram-unit", 0, "Unit of N-gram for input text"},
        {"ensemble_simstring_ngram_unit", -1, {"ensemble", "simstring_ngram_unit"}, "ensemble-simstring-ngram-unit", 0, "Unit of N-gram for romaji notation of input text"},
        {"svr_simstring_ngram_unit", -1, {"svr", "simstring_ngram_unit"}, "svr-simstring-ngram-unit", 0, "Unit of N-gram for romaji notation of input text"},
        {"svr_features_path", "features.tsv", {"svr", "features_path"}, "svr-features-path", 0, "feature definition file for support vector regression"},
        {"svr_patterns_home", ".", {"svr", "patterns_home"}, "svr-patterns-home", 0, "directory for pattern files for regular expression-based feature extractors"},
        {"corpus_path", "", {"common", "corpus_path"}},
        {"id_col", 0, {"common", "id_col"}, "id-col", 0, "column number (starts with 1) of ID in corpus rows. ignored if id_col==0"},
        {"text_col", 1, {"common", "text_col"}, "text-col", 0, "column mumber of text in corpus rows"},
        {"features_col", 0, {"common", "features_col"}, "features-col", 0, "column number of features in corpus rows"},
        {"normalize_text", false, {"icu", "enabled"}, "normalize-text", 0, "enable text normalization"},
        {"icu_normalization_dir", "", {"icu", "normalization", "dir"}, "icu-normalization-dir", 0, "directory for ICU normalizer configuration file"},
        {"icu_normalization_name", "", {"icu", "normalization", "name"}, "icu-normalization-name", 0, "file name of ICU normalizer configuration file"},
        {"icu_predefined_normalizer", "", {"icu", "normalization", "predefined_normalizer"}, "icu-predefined-normalizer", 0, "name of predefined ICU normalizer"},
        {"icu_transliteration_path", "", {"icu", "transliteration", "path"}, "icu-transliteration-path", 0, "path for configuration file of transliterator"},
        {"icu_to_lower", "", {"icu", "to_lower"}, "icu-to-lower", 0, "convert input texts to lowercase"},
        {"verbose", false, {"common", "verbose"}, "verbose", 'v', "show more information"},
        {"conf_path", "", "config", 'c', "config file path"}
    };
    paramset::manager pm(defs);

    try{
        pm.load(argc, argv, "config");

        if(!pm.rest.empty()){
            pm["corpus_path"] = pm.rest.front().as<std::string>();
        }
        if(pm.get<std::string>("corpus_path").empty()){
            throw std::invalid_argument("no corpus file specified");
        }
        std::string corpus_path = pm.get<std::string>("corpus_path");

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

        auto resembla_measures = split_to_resembla_measures(pm.get<std::string>("resembla_measure"));
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

        if(pm.get<bool>("verbose")){
            std::cerr << "Configurations:" << std::endl;
            std::cerr << "  Common:" << std::endl;
            std::cerr << "    corpus_path=" << pm.get<std::string>("corpus_path") << std::endl;
            std::cerr << "    text_col=" << pm.get<int>("text_col") << std::endl;
            std::cerr << "    features_col=" << pm.get<int>("features_col") << std::endl;
            std::cerr << "  SimString:" << std::endl;
            std::cerr << "    ngram_unit=" << pm.get<int>("simstring_ngram_unit") << std::endl;
            if(pm.get<bool>("normalize_text")){
                std::cerr << "  ICU:" << std::endl;
                std::cerr << "    normalization_dir=" << pm.get<std::string>("icu_normalization_dir") << std::endl;
                std::cerr << "    normalization_name=" << pm.get<std::string>("icu_normalization_name") << std::endl;
                std::cerr << "    predefined_normalizer=" << pm.get<std::string>("icu_predefined_normalizer") << std::endl;
                std::cerr << "    transliteration_path=" << pm.get<std::string>("icu_transliteration_path") << std::endl;
                std::cerr << "    to_lower=" << (pm.get<bool>("icu_to_lower") ? "true" : "false")<< std::endl;
            }
            if(use_ensemble){
                std::cerr << "  measure=" << STR(ensemble) << std::endl;
                std::cerr << "    simstring_ngram_unit=" << pm.get<int>("ensemble_simstring_ngram_unit") << std::endl;
            }
            for(auto resembla_measure: resembla_measures){
                if(resembla_measure == edit_distance){
                    std::cerr << "  measure=" << STR(edit_distance) << std::endl;
                    std::cerr << "    simstring_ngram_unit=" << pm.get<int>("ed_simstring_ngram_unit") << std::endl;
                }
                else if(resembla_measure == weighted_word_edit_distance){
                    std::cerr << "  measure=" << STR(weighted_word_edit_distance) << std::endl;
                    std::cerr << "    simstring_ngram_unit=" << pm.get<int>("wwed_simstring_ngram_unit") << std::endl;
                    std::cerr << "    mecab_options=" << pm.get<std::string>("wwed_mecab_options") << std::endl;
                }
                else if(resembla_measure == weighted_pronunciation_edit_distance){
                    std::cerr << "  measure=" << STR(weighted_pronunciation_edit_distance) << std::endl;
                    std::cerr << "    simstring_ngram_unit=" << pm.get<int>("wped_simstring_ngram_unit") << std::endl;
                    std::cerr << "    mecab_options=" << pm.get<std::string>("wped_mecab_options") << std::endl;
                    std::cerr << "    mecab_feature_pos=" << pm.get<int>("wped_mecab_feature_pos") << std::endl;
                    std::cerr << "    mecab_pronunciation_of_marks=" << pm.get<std::string>("wped_mecab_pronunciation_of_marks") << std::endl;
                    std::cerr << "    base_weight=" << pm.get<double>("wped_base_weight") << std::endl;
                    std::cerr << "    delete_insert_ratio=" << pm.get<double>("wped_delete_insert_ratio") << std::endl;
                    std::cerr << "    letter_weight_path=" << pm.get<std::string>("wped_letter_weight_path") << std::endl;
                }
                else if(resembla_measure == weighted_romaji_edit_distance){
                    std::cerr << "  measure=" << STR(weighted_romaji_edit_distance) << std::endl;
                    std::cerr << "    simstring_ngram_unit=" << pm.get<int>("wred_simstring_ngram_unit") << std::endl;
                    std::cerr << "    mecab_options=" << pm.get<std::string>("wred_mecab_options") << std::endl;
                    std::cerr << "    mecab_feature_pos=" << pm.get<int>("wred_mecab_feature_pos") << std::endl;
                    std::cerr << "    mecab_pronunciation_of_marks=" << pm.get<std::string>("wred_mecab_pronunciation_of_marks") << std::endl;
                }
                else if(resembla_measure == keyword_match){
                    std::cerr << "  measure=" << STR(keyword_match) << std::endl;
                    std::cerr << "    simstring_ngram_unit=" << pm.get<int>("km_simstring_ngram_unit") << std::endl;
                }
                else if(resembla_measure == svr){
                    std::cerr << "  measure=" << STR(svr) << std::endl;
                    std::cerr << "    simstring_ngram_unit=" << pm.get<int>("svr_simstring_ngram_unit") << std::endl;
                    std::cerr << "    features_path=" << pm.get<std::string>("svr_features_path") << std::endl;
                    std::cerr << "    patterns_home=" << pm.get<std::string>("svr_patterns_home") << std::endl;
                }
            }
        }

        std::shared_ptr<StringNormalizer> normalize;
        if(pm.get<bool>("normalize_text")){
            normalize = std::make_shared<StringNormalizer>(
                pm.get<std::string>("icu_normalization_dir"),
                pm.get<std::string>("icu_normalization_name"),
                pm.get<std::string>("icu_predefined_normalizer"),
                pm.get<std::string>("icu_transliteration_path"),
                pm.get<bool>("icu_to_lower"));
        }

        for(auto resembla_measure: resembla_measures){
            std::string db_path = db_path_from_resembla_measure(corpus_path, resembla_measure);
            std::string index_path = inverse_path_from_resembla_measure(corpus_path, resembla_measure);

            if(resembla_measure == edit_distance){
                auto preprocessor = std::make_shared<AsIsPreprocessor<string_type>>();
                create_index(corpus_path, db_path, index_path, pm.get<int>("ed_simstring_ngram_unit"),
                        preprocessor, preprocessor, pm.get<int>("text_col"), pm.get<int>("features_col"), normalize);
            }
            else if(resembla_measure == weighted_word_edit_distance){
                auto indexer = std::make_shared<AsIsPreprocessor<string_type>>();
                auto preprocessor = std::make_shared<WeightedSequenceBuilder<WordPreprocessor<string_type>, WordWeight>>(
                    std::make_shared<WordPreprocessor<string_type>>(pm.get<std::string>("wwed_mecab_options")),
                    std::make_shared<WordWeight>(pm.get<double>("wwed_base_weight"),
                        pm.get<double>("wwed_delete_insert_ratio"), pm.get<double>("wwed_noun_coefficient"),
                        pm.get<double>("wwed_verb_coefficient"), pm.get<double>("wwed_adj_coefficient")));
                create_index(corpus_path, db_path, index_path, pm.get<int>("wwed_simstring_ngram_unit"),
                        indexer, preprocessor, pm.get<int>("text_col"), pm.get<int>("features_col"), normalize);
            }
            else if(resembla_measure == weighted_pronunciation_edit_distance){
                auto indexer = std::make_shared<PronunciationPreprocessor>(pm.get<std::string>("wped_mecab_options"),
                        pm.get<int>("wped_mecab_feature_pos"), pm.get<std::string>("wped_mecab_pronunciation_of_marks"));
                auto preprocessor = std::make_shared<WeightedSequenceBuilder<PronunciationPreprocessor, LetterWeight<string_type>>>(
                    indexer,
                    std::make_shared<LetterWeight<string_type>>(pm.get<double>("wped_base_weight"),
                        pm.get<double>("wped_delete_insert_ratio"), pm.get<std::string>("wped_letter_weight_path")));
                create_index(corpus_path, db_path, index_path, pm.get<int>("wped_simstring_ngram_unit"),
                        indexer, preprocessor, pm.get<int>("text_col"), pm.get<int>("features_col"), normalize);
            }
            else if(resembla_measure == weighted_romaji_edit_distance){
                auto indexer = std::make_shared<RomajiPreprocessor>(pm.get<std::string>("wred_mecab_options"),
                    pm.get<int>("wred_mecab_feature_pos"), pm.get<std::string>("wred_mecab_pronunciation_of_marks"));
                auto preprocessor = std::make_shared<WeightedSequenceBuilder<RomajiPreprocessor, RomajiWeight>>(
                    indexer,
                    std::make_shared<RomajiWeight>(pm.get<double>("wred_base_weight"), pm.get<double>("wred_delete_insert_ratio"),
                        pm.get<double>("wred_uppercase_coefficient"), pm.get<double>("wred_lowercase_coefficient"),
                        pm.get<double>("wred_vowel_coefficient"), pm.get<double>("wred_consonant_coefficient")));
                create_index(corpus_path, db_path, index_path, pm.get<int>("wred_simstring_ngram_unit"),
                        indexer, preprocessor, pm.get<int>("text_col"), pm.get<int>("features_col"), normalize);
            }
            else if(resembla_measure == keyword_match){
                auto indexer = std::make_shared<AsIsPreprocessor<string_type>>();
                auto preprocessor = std::make_shared<KeywordMatchPreprocessor<RomajiPreprocessor>>(
                    std::make_shared<RomajiPreprocessor>(pm.get<std::string>("index_romaji_mecab_options"),
                        pm.get<int>("index_romaji_mecab_feature_pos"),
                        pm.get<std::string>("index_romaji_mecab_pronunciation_of_marks")));
                create_index(corpus_path, db_path, index_path, pm.get<int>("km_simstring_ngram_unit"),
                        indexer, preprocessor, pm.get<int>("text_col"), pm.get<int>("features_col"), normalize);
            }
            else if(resembla_measure == svr){
                auto indexer = std::make_shared<RomajiPreprocessor>(pm.get<std::string>("index_romaji_mecab_options"),
                    pm.get<int>("index_romaji_mecab_feature_pos"),
                    pm.get<std::string>("index_romaji_mecab_pronunciation_of_marks"));

                auto features = load_features(pm.get<std::string>("svr_features_path"));
                if(features.empty()){
                    throw std::runtime_error("no feature");
                }
                const auto& base_feature = features[0][0];

                auto extractor = std::make_shared<FeatureExtractor>();
                for(const auto& feature: features){
                    const auto& name = feature[0];
                    if(name == base_feature){
                        continue;
                    }

                    const auto& feature_extractor_type = feature[1];
                    if(feature_extractor_type == "re"){
                        extractor->append(name, std::make_shared<RegexFeatureExtractor>(
                                pm.get<std::string>("svr_patterns_home") + "/" + name + ".tsv"));
                    }
                    else if(feature_extractor_type == "date_period"){
                        extractor->append(name, std::make_shared<DatePeriodFeatureExtractor>());
                    }
                    else if(feature_extractor_type == "time_period"){
                        extractor->append(name, std::make_shared<TimePeriodFeatureExtractor>());
                    }
                    else if(feature_extractor_type != "-"){
                        throw std::runtime_error("unknown feature extractor type: " + feature_extractor_type);
                    }
                }
                create_index(corpus_path, db_path, index_path, pm.get<int>("simstring_ngram_unit"),
                        indexer, extractor, pm.get<int>("text_col"), pm.get<int>("features_col"), normalize);
            }

            std::cerr << "index saved to " << index_path << std::endl;
        }

        if(use_ensemble){
            std::string db_path = db_path_from_resembla_measure(corpus_path, ensemble);
            std::string index_path = inverse_path_from_resembla_measure(corpus_path, ensemble);

            auto indexer = std::make_shared<RomajiPreprocessor>(pm.get<std::string>("index_romaji_mecab_options"),
                    pm.get<int>("index_romaji_mecab_feature_pos"),
                    pm.get<std::string>("index_romaji_mecab_pronunciation_of_marks"));
            create_index(corpus_path, db_path, index_path, pm.get<int>("wred_simstring_ngram_unit"),
                    indexer, std::shared_ptr<RomajiPreprocessor>(), pm.get<int>("text_col"), pm.get<int>("features_col"), normalize);

            std::cerr << "index saved to " << index_path << std::endl;
        }
    }
    catch(const std::exception& e){
        std::cerr << "error: " << e.what() << std::endl;
        exit(1);
    }

    return 0;
}
