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

#include "paramset.hpp"
#include <resembla/resembla_util.hpp>

#include <resembla/simstring/simstring.h>

#include <resembla/measure/word_sequence_builder.hpp>
#include <resembla/measure/pronunciation_sequence_builder.hpp>
#include <resembla/measure/romaji_sequence_builder.hpp>
#include <resembla/measure/keyword_match_preprocessor.hpp>

using namespace resembla;

template<typename Preprocessor>
void create_index(const std::string corpus_path, const std::string db_path, const std::string inverse_path,
        int n, Preprocessor preprocess, size_t extra_col = 0)
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
        auto original = columns[0];
        auto s = preprocess.index(original);

        if(extra_col > 0 && extra_col - 1 < columns.size()){
            original += L"\t" + columns[extra_col - 1];
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
            ofs << p.first << L'\t' << original << std::endl;
        }
    }
}

int main(int argc, char* argv[])
{
    init_locale();

    paramset::definitions defs = {
        {"simstring_ngram_unit", 2, {"simstring", "ngram_unit"}, "simstring-ngram-unit", 'N', "Unit of N-gram for SimString"},
        {"simstring_text_preprocess", "asis", {"simstring", "text_preprocess"}, "simstring-text-preprocess", 'P', "preprocessing method for texts to create index"},
        {"ed_simstring_ngram_unit", -1, {"edit_distance", "simstring_ngram_unit"}, "ed-simstring-ngram-unit", 0, "Unit of N-gram for input text"},
        {"wwed_simstring_ngram_unit", -1, {"weighted_word_edit_distance", "simstring_ngram_unit"}, "wwed-simstring-ngram-unit", 0, "Unit of N-gram for input text"},
        {"wwed_mecab_options", "", {"weighted_word_edit_distance", "mecab_options"}, "wwed-mecab-options", 0, "MeCab options for weighted word edit distance"},
        {"wped_simstring_ngram_unit", -1, {"weighted_pronunciation_edit_distance", "simstring_ngram_unit"}, "wped-simstring-ngram-unit", 0, "Unit of N-gram for pronunciation of input text"},
        {"wped_mecab_options", "", {"weighted_pronunciation_edit_distance", "mecab_options"}, "wped-mecab-options", 0, "MeCab options for weighted pronunciation edit distance"},
        {"wped_mecab_feature_pos", 7, {"weighted_pronunciation_edit_distance", "mecab_feature_pos"}, "wped-mecab-feature-pos", 0, "Position of pronunciation in feature for weighted pronunciation edit distance"},
        {"wped_mecab_pronunciation_of_marks", "", {"weighted_pronunciation_edit_distance", "mecab_pronunciation_of_marks"}, "wped-mecab-pronunciation-of-marks", 0, "pronunciation in MeCab features when input is a mark"},
        {"wred_simstring_ngram_unit", -1, {"weighted_romaji_edit_distance", "simstring_ngram_unit"}, "wred-simstring-ngram-unit", 0, "Unit of N-gram for romaji notation of input text"},
        {"wred_mecab_options", "", {"weighted_romaji_edit_distance", "mecab_options"}, "wred-mecab-options", 0, "MeCab options for weighted romaji edit distance"},
        {"wred_mecab_feature_pos", 7, {"weighted_romaji_edit_distance", "mecab_feature_pos"}, "wred-mecab-feature-pos", 0, "Position of pronunciation in feature for weighted romaji edit distance"},
        {"wred_mecab_pronunciation_of_marks", "", {"weighted_romaji_edit_distance", "mecab_pronunciation_of_marks"}, "wred-mecab-pronunciation-of-marks", 0, "pronunciation in MeCab features when input is a mark"},
        {"corpus_path", "", {"common", "corpus_path"}},
        {"feature_col", 0, {"common", "feature_col"}, "feature-col", 0, "index of feature column in corpus rows"},
        {"varbose", false, {"common", "varbose"}, 'v', "show more information"},
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
        auto simstring_text_preprocesses = split_to_text_preprocesses(pm.get<std::string>("simstring_text_preprocess"));

        if(pm.get<bool>("varbose")){
            std::cerr << "Configurations:" << std::endl;
            std::cerr << "  Common:" << std::endl;
            std::cerr << "    corpus_path=" << corpus_path << std::endl;
            std::cerr << "  SimString:" << std::endl;
            std::cerr << "    ngram_unit=" << default_simstring_ngram_unit << std::endl;
            for(auto simstring_text_preprocess: simstring_text_preprocesses){
                if(simstring_text_preprocess == asis){
                    std::cerr << "  text_preprocess=" << STR(asis) << std::endl;
                    std::cerr << "    simstring_ngram_unit=" << wwed_simstring_ngram_unit << std::endl;
                    std::cerr << "    mecab_options=" << pm.get<std::string>("wwed_mecab_options") << std::endl;
                }
                else if(simstring_text_preprocess == pronunciation){
                    std::cerr << "  text_preprocess=" << STR(pronunciation) << std::endl;
                    std::cerr << "    simstring_ngram_unit=" << wped_simstring_ngram_unit << std::endl;
                    std::cerr << "    mecab_options=" << pm.get<std::string>("wped_mecab_options") << std::endl;
                    std::cerr << "    mecab_feature_pos=" << pm.get<int>("wped_mecab_feature_pos") << std::endl;
                    std::cerr << "    mecab_pronunciation_of_marks=" << pm.get<std::string>("wped_mecab_pronunciation_of_marks") << std::endl;
                }
                else if(simstring_text_preprocess == romaji){
                    std::cerr << "  text_preprocess=" << STR(romaji) << std::endl;
                    std::cerr << "    simstring_ngram_unit=" << wred_simstring_ngram_unit << std::endl;
                    std::cerr << "    mecab_options=" << pm.get<std::string>("wred_mecab_options") << std::endl;
                    std::cerr << "    mecab_feature_pos=" << pm.get<int>("wred_mecab_feature_pos") << std::endl;
                    std::cerr << "    mecab_pronunciation_of_marks=" << pm.get<std::string>("wred_mecab_pronunciation_of_marks") << std::endl;
                }
            }
        }

        for(auto simstring_text_preprocess: simstring_text_preprocesses){
            std::string db_path = db_path_from_simstring_text_preprocess(corpus_path, simstring_text_preprocess);
            std::string inverse_path = inverse_path_from_simstring_text_preprocess(corpus_path, simstring_text_preprocess);

            if(simstring_text_preprocess == asis){
                WordSequenceBuilder builder(pm.get<std::string>("wwed_mecab_options"));
                create_index(corpus_path, db_path, inverse_path, wwed_simstring_ngram_unit, builder, 0);
            }
            else if(simstring_text_preprocess == pronunciation){
                PronunciationSequenceBuilder builder(pm.get<std::string>("wped_mecab_options"),
                        pm.get<int>("wped_mecab_feature_pos"), pm.get<std::string>("wped_mecab_pronunciation_of_marks"));
                create_index(corpus_path, db_path, inverse_path, wped_simstring_ngram_unit, builder, 0);
            }
            else if(simstring_text_preprocess == romaji){
                RomajiSequenceBuilder builder(pm.get<std::string>("wred_mecab_options"),
                        pm.get<int>("wred_mecab_feature_pos"), pm.get<std::string>("wred_mecab_pronunciation_of_marks"));
                create_index(corpus_path, db_path, inverse_path, wred_simstring_ngram_unit, builder, 0);
            }
            else if(simstring_text_preprocess == keyword){
                create_index(corpus_path, db_path, inverse_path, wred_simstring_ngram_unit,
                        KeywordMatchPreprocessor<string_type>(), pm.get<int>("feature_col"));
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
