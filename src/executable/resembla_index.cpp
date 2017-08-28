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
#include <unordered_map>
#include <set>
#include <stdexcept>

#include <paramset.hpp>
#include <resembla/resembla_util.hpp>

#include <resembla/simstring/simstring.h>
#include <json.hpp>

#include <resembla/measure/asis_sequence_builder.hpp>
#include <resembla/measure/word_sequence_builder.hpp>
#include <resembla/measure/pronunciation_sequence_builder.hpp>
#include <resembla/measure/romaji_sequence_builder.hpp>
#include <resembla/measure/keyword_match_preprocessor.hpp>

#include <resembla/measure/weighted_sequence_builder.hpp>

#include <resembla/measure/feature_match_weight.hpp>
#include <resembla/measure/surface_match_cost.hpp>

#include <resembla/measure/romaji_match_weight.hpp>
#include <resembla/measure/romaji_match_cost.hpp>

#include <resembla/measure/weighted_sequence_serializer.hpp>

#include <resembla/regression/extractor/feature_extractor.hpp>
#include <resembla/regression/extractor/regex_feature_extractor.hpp>
#include <resembla/regression/extractor/date_period_feature_extractor.hpp>
#include <resembla/regression/extractor/time_period_feature_extractor.hpp>

using namespace resembla;

template<typename Preprocessor>
void create_index(const std::string corpus_path, const std::string db_path, const std::string inverse_path,
        int n, Preprocessor preprocess, size_t text_col, size_t features_col = 0)
{
    simstring::ngram_generator gen(n, false);
    simstring::writer_base<string_type> dbw(gen, db_path);
    std::unordered_map<string_type, std::set<string_type>> inserted;
    std::basic_ifstream<string_type::value_type> ifs(corpus_path);
    if(ifs.fail()){
        throw std::runtime_error("input file is not available: " + corpus_path);
    }
    while(ifs.good()){
        string_type line;
        std::getline(ifs, line);
        if(ifs.eof() || line.length() == 0){
            break;
        }

        auto columns = split(line, L'\t');
        if(text_col > columns.size()){
            continue;
        }

        auto original = columns[text_col - 1];
        auto s = preprocess.index(original);

        if(features_col > 0 && features_col - 1 < columns.size()){
            original += L"\t" + columns[features_col - 1];
        }

        if(inserted.count(s) == 0){
            dbw.insert(s);
            inserted[s] = {original};
        }
        else{
            inserted[s].insert(original);
        }
    }
    dbw.close();
    std::basic_ofstream<string_type::value_type> ofs;
    ofs.open(inverse_path);
    for(auto p: inserted){
        for(auto original: p.second){
            nlohmann::json j = preprocess(original, true);
            auto preprocessed = cast_string<string_type>(j.dump());
            auto columns = split(original, L'\t');
            ofs << p.first << L'\t' << columns[0] << L'\t' << preprocessed << std::endl;
        }
    }
}

int main(int argc, char* argv[])
{
    init_locale();

    paramset::definitions defs = {
        {"simstring_ngram_unit", 2, {"simstring", "ngram_unit"}, "simstring-ngram-unit", 'N', "Unit of N-gram for SimString"},
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
        {"svr_features_path", "features.tsv", {"svr", "features_path"}, "svr-features-path", 0, "feature definition file for support vector regression"},
        {"svr_patterns_home", ".", {"svr", "patterns_home"}, "svr-patterns-home", 0, "directory for pattern files for regular expression-based feature extractors"},
        {"corpus_path", "", {"common", "corpus_path"}},
        {"text_col", 1, {"common", "text_col"}, "text-col", 0, "column mumber of text in corpus rows"},
        {"features_col", 0, {"common", "features_col"}, "features-col", 0, "column number of features in corpus rows"},
        {"verbose", false, {"common", "verbose"}, 'v', "show more information"},
        {"conf_path", "", "config", 'c', "config file path"}
    };
    paramset::manager pm(defs);

    try{
        pm.load(argc, argv, "config");

        std::string corpus_path = read_value_with_rest(pm, "corpus_path", ""); // must not be empty
        int default_simstring_ngram_unit = pm["simstring_ngram_unit"];
        int wwed_simstring_ngram_unit = pm.get<int>("wwed_simstring_ngram_unit") != -1 ?
            pm.get<int>("wwed_simstring_ngram_unit") : default_simstring_ngram_unit;
        int wped_simstring_ngram_unit = pm.get<int>("wped_simstring_ngram_unit") != -1 ?
            pm.get<int>("wped_simstring_ngram_unit") : default_simstring_ngram_unit;
        int wred_simstring_ngram_unit = pm.get<int>("wred_simstring_ngram_unit") != -1 ?
            pm.get<int>("wred_simstring_ngram_unit") : default_simstring_ngram_unit;
        int km_simstring_ngram_unit = pm.get<int>("km_simstring_ngram_unit") != -1 ?
            pm.get<int>("km_simstring_ngram_unit") : default_simstring_ngram_unit;
        auto resembla_measures = split_to_resembla_measures(pm.get<std::string>("resembla_measure"));

        if(pm.get<bool>("verbose")){
            std::cerr << "Configurations:" << std::endl;
            std::cerr << "  Common:" << std::endl;
            std::cerr << "    corpus_path=" << corpus_path << std::endl;
            std::cerr << "    text_col=" << pm.get<int>("text_col") << std::endl;
            std::cerr << "    features_col=" << pm.get<int>("features_col") << std::endl;
            std::cerr << "  SimString:" << std::endl;
            std::cerr << "    ngram_unit=" << default_simstring_ngram_unit << std::endl;
            for(auto resembla_measure: resembla_measures){
                if(resembla_measure == edit_distance){
                    std::cerr << "  measure=" << STR(edit_distance) << std::endl;
                    std::cerr << "    simstring_ngram_unit=" << wwed_simstring_ngram_unit << std::endl;
                    //std::cerr << "    mecab_options=" << pm.get<std::string>("wwed_mecab_options") << std::endl;
                }
                if(resembla_measure == weighted_word_edit_distance){
                    std::cerr << "  measure=" << STR(weighted_word_edit_distance) << std::endl;
                    std::cerr << "    simstring_ngram_unit=" << wwed_simstring_ngram_unit << std::endl;
                    std::cerr << "    mecab_options=" << pm.get<std::string>("wwed_mecab_options") << std::endl;
                }
                else if(resembla_measure == weighted_pronunciation_edit_distance){
                    std::cerr << "  measure=" << STR(weighted_pronunciation_edit_distance) << std::endl;
                    std::cerr << "    simstring_ngram_unit=" << wped_simstring_ngram_unit << std::endl;
                    std::cerr << "    mecab_options=" << pm.get<std::string>("wped_mecab_options") << std::endl;
                    std::cerr << "    mecab_feature_pos=" << pm.get<int>("wped_mecab_feature_pos") << std::endl;
                    std::cerr << "    mecab_pronunciation_of_marks=" << pm.get<std::string>("wped_mecab_pronunciation_of_marks") << std::endl;
                }
                else if(resembla_measure == weighted_romaji_edit_distance){
                    std::cerr << "  measure=" << STR(weighted_romaji_edit_distance) << std::endl;
                    std::cerr << "    simstring_ngram_unit=" << wred_simstring_ngram_unit << std::endl;
                    std::cerr << "    mecab_options=" << pm.get<std::string>("wred_mecab_options") << std::endl;
                    std::cerr << "    mecab_feature_pos=" << pm.get<int>("wred_mecab_feature_pos") << std::endl;
                    std::cerr << "    mecab_pronunciation_of_marks=" << pm.get<std::string>("wred_mecab_pronunciation_of_marks") << std::endl;
                }
                else if(resembla_measure == keyword_match){
                    std::cerr << "  measure=" << STR(keyword_match) << std::endl;
                    std::cerr << "    simstring_ngram_unit=" << km_simstring_ngram_unit << std::endl;
                }
                else if(resembla_measure == svr){
                    std::cerr << "  measure=" << STR(keyword) << std::endl;
                    std::cerr << "    simstring_ngram_unit=" << km_simstring_ngram_unit << std::endl;
                }
            }
        }

        for(auto resembla_measure: resembla_measures){
            std::string db_path = db_path_from_resembla_measure(corpus_path, resembla_measure);
            std::string inverse_path = inverse_path_from_resembla_measure(corpus_path, resembla_measure);

            if(resembla_measure == edit_distance){
                AsIsSequenceBuilder<string_type> builder;
                create_index(corpus_path, db_path, inverse_path, wwed_simstring_ngram_unit, builder,
                        pm.get<int>("text_col"), pm.get<int>("features_col"));
            }
            else if(resembla_measure == weighted_word_edit_distance){
                WeightedSequenceBuilder<WordSequenceBuilder, FeatureMatchWeight> builder(
                    WordSequenceBuilder(pm.get<std::string>("wwed_mecab_options")),
                    FeatureMatchWeight(pm.get<double>("wwed_base_weight"),
                        pm.get<double>("wwed_delete_insert_ratio"), pm.get<double>("wwed_noun_coefficient"),
                        pm.get<double>("wwed_verb_coefficient"), pm.get<double>("wwed_adj_coefficient")));
                create_index(corpus_path, db_path, inverse_path, wwed_simstring_ngram_unit, builder,
                        pm.get<int>("text_col"), pm.get<int>("features_col"));
            }
            else if(resembla_measure == weighted_pronunciation_edit_distance){
                PronunciationSequenceBuilder builder(pm.get<std::string>("wped_mecab_options"),
                        pm.get<int>("wped_mecab_feature_pos"), pm.get<std::string>("wped_mecab_pronunciation_of_marks"));
                create_index(corpus_path, db_path, inverse_path, wped_simstring_ngram_unit,
                        builder, pm.get<int>("text_col"), pm.get<int>("features_col"));
            }
            else if(resembla_measure == weighted_romaji_edit_distance){
                WeightedSequenceBuilder<RomajiSequenceBuilder, RomajiMatchWeight> builder(
                    RomajiSequenceBuilder(pm.get<std::string>("wred_mecab_options"),
                        pm.get<int>("wred_mecab_feature_pos"), pm.get<std::string>("wred_mecab_pronunciation_of_marks")),
                    RomajiMatchWeight(pm.get<double>("wred_base_weight"), pm.get<double>("wred_delete_insert_ratio"),
                        pm.get<double>("wred_uppercase_coefficient"), pm.get<double>("wred_lowercase_coefficient"),
                        pm.get<double>("wred_vowel_coefficient"), pm.get<double>("wred_consonant_coefficient")));
                create_index(corpus_path, db_path, inverse_path, wred_simstring_ngram_unit,
                        builder, pm.get<int>("text_col"), pm.get<int>("features_col"));
            }
            else if(resembla_measure == keyword_match){
                create_index(corpus_path, db_path, inverse_path, km_simstring_ngram_unit,
                        KeywordMatchPreprocessor<string_type>(), pm.get<int>("text_col"), pm.get<int>("features_col"));
            }
            else if(resembla_measure == svr){
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
                create_index(corpus_path, db_path, inverse_path, wred_simstring_ngram_unit,
                        extractor, pm.get<int>("text_col"), pm.get<int>("features_col"));
            }

            std::cerr << "database saved to " << db_path << std::endl;
        }
    }
    catch(const std::exception& e){
        std::cerr << "error: " << e.what() << std::endl;
        exit(1);
    }

    return 0;
}
