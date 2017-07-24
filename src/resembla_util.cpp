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

#include <locale>
#include <codecvt>

#include "simstring/simstring.h"

#include "resembla_util.hpp"

#include "measure/edit_distance.hpp"
#include "measure/weighted_edit_distance.hpp"

#include "measure/asis_sequence_builder.hpp"
#include "measure/weighted_sequence_builder.hpp"

#include "measure/word_sequence_builder.hpp"
#include "measure/feature_match_weight.hpp"
#include "measure/surface_match_cost.hpp"

#include "measure/pronunciation_sequence_builder.hpp"
#include "measure/uniform_cost.hpp"

#include "measure/romaji_sequence_builder.hpp"
#include "measure/romaji_match_weight.hpp"
#include "measure/romaji_match_cost.hpp"

namespace resembla {

const std::string SIMSTRING_DB_FILE_SUFFIX = ".simstring.cdb";
const std::string SIMSTRING_DB_FILE_COMMON_SUFFIX = ".simstring_db.";
const std::string SIMSTRING_INVERSE_FILE_COMMON_SUFFIX = ".inverse.";

// utility function for converting string that represents a simstring measure to int
int simstring_measure_from_string(const std::string& simstring_measure_str)
{
    if(simstring_measure_str == "exact"){
        return simstring::exact;
    }
    else if(simstring_measure_str == "dice"){
        return simstring::dice;
    }
    else if(simstring_measure_str == "cosine"){
        return  simstring::cosine;
    }
    else if(simstring_measure_str == "jaccard"){
        return  simstring::jaccard;
    }
    else if(simstring_measure_str == "overlap"){
        return simstring::overlap;
    }
    else{
        throw std::invalid_argument("unknown simstring measure: " + simstring_measure_str);
    }
}

std::string read_value_with_rest(paramset::manager& pm, const std::string key, const char* throw_if)
{
    return read_value_with_rest(pm, key, std::string(throw_if));
}

std::string db_path_from_simstring_text_preprocess(const std::string& corpus_path, const text_preprocess simstring_text_preprocess)
{
    if(simstring_text_preprocess == asis){
        return corpus_path + SIMSTRING_DB_FILE_COMMON_SUFFIX + STR(asis);
    }
    else if(simstring_text_preprocess == pronunciation){
        return corpus_path + SIMSTRING_DB_FILE_COMMON_SUFFIX + STR(pronunciation);
    }
    else if(simstring_text_preprocess == romaji){
        return corpus_path + SIMSTRING_DB_FILE_COMMON_SUFFIX + STR(romaji);
    }
    else{
        throw std::invalid_argument("unknown preprocessing method: " + std::string(STR(simstring_text_preprocess)));
    }
}

std::string inverse_path_from_simstring_text_preprocess(const std::string& corpus_path, const text_preprocess simstring_text_preprocess)
{
    if(simstring_text_preprocess == asis){
        return corpus_path + SIMSTRING_INVERSE_FILE_COMMON_SUFFIX + STR(asis);
    }
    else if(simstring_text_preprocess == pronunciation){
        return corpus_path + SIMSTRING_INVERSE_FILE_COMMON_SUFFIX + STR(pronunciation);
    }
    else if(simstring_text_preprocess == romaji){
        return corpus_path + SIMSTRING_INVERSE_FILE_COMMON_SUFFIX + STR(romaji);
    }
    else{
        throw std::invalid_argument("unknown preprocessing method: " + std::string(STR(simstring_text_preprocess)));
    }
}

std::string db_path_from_resembla_measure(const std::string& corpus_path, const measure resembla_measure)
{
    if(resembla_measure == edit_distance){
        return corpus_path + SIMSTRING_DB_FILE_COMMON_SUFFIX + STR(asis);
    }
    else if(resembla_measure == weighted_word_edit_distance){
        return corpus_path + SIMSTRING_DB_FILE_COMMON_SUFFIX + STR(asis);
    }
    else if(resembla_measure == weighted_pronunciation_edit_distance){
        return corpus_path + SIMSTRING_DB_FILE_COMMON_SUFFIX + STR(pronunciation);
    }
    else if(resembla_measure == weighted_romaji_edit_distance){
        return corpus_path + SIMSTRING_DB_FILE_COMMON_SUFFIX + STR(romaji);
    }
    else{
        throw std::invalid_argument("unknown Resembla measure: " + std::string(STR(resembla_measure)));
    }
}

std::string inverse_path_from_resembla_measure(const std::string& corpus_path, const measure resembla_measure)
{
    if(resembla_measure == edit_distance){
        return corpus_path + SIMSTRING_INVERSE_FILE_COMMON_SUFFIX + STR(asis);
    }
    else if(resembla_measure == weighted_word_edit_distance){
        return corpus_path + SIMSTRING_INVERSE_FILE_COMMON_SUFFIX + STR(asis);
    }
    else if(resembla_measure == weighted_pronunciation_edit_distance){
        return corpus_path + SIMSTRING_INVERSE_FILE_COMMON_SUFFIX + STR(pronunciation);
    }
    else if(resembla_measure == weighted_romaji_edit_distance){
        return corpus_path + SIMSTRING_INVERSE_FILE_COMMON_SUFFIX + STR(romaji);
    }
    else{
        throw std::invalid_argument("unknown Resembla measure: " + std::string(STR(resembla_measure)));
    }
}

std::vector<text_preprocess> split_to_text_preprocesses(std::string text, char delimiter, bool ignore_unknown_measure)
{
    std::vector<text_preprocess> result;
    for(auto text_preprocess_str: split(text, delimiter)){
        if(text_preprocess_str == STR(asis)){
            result.push_back(asis);
        }
        else if(text_preprocess_str == STR(pronunciation)){
            result.push_back(pronunciation);
        }
        else if(text_preprocess_str == STR(romaji)){
            result.push_back(romaji);
        }
        else{
            if(!ignore_unknown_measure){
                throw std::invalid_argument("unknown text_preprocess: " + text_preprocess_str);
            }
        }
    }
    return result;
}

std::vector<measure> split_to_resembla_measures(std::string text, char delimiter, bool ignore_unknown_measure)
{
    std::vector<measure> result;
    for(auto resembla_measure_str: split(text, delimiter)){
        if(resembla_measure_str == STR(edit_distance)){
            result.push_back(edit_distance);
        }
        else if(resembla_measure_str == STR(weighted_word_edit_distance)){
            result.push_back(weighted_word_edit_distance);
        }
        else if(resembla_measure_str == STR(weighted_pronunciation_edit_distance)){
            result.push_back(weighted_pronunciation_edit_distance);
        }
        else if(resembla_measure_str == STR(weighted_romaji_edit_distance)){
            result.push_back(weighted_romaji_edit_distance);
        }
        else{
            if(!ignore_unknown_measure){
                throw std::invalid_argument("unknown resembla_measure: " + resembla_measure_str);
            }
        }
    }
    return result;
}

ResemblaEnsemble construct_resembla_ensemble(std::string corpus_path, paramset::manager& pm)
{
    int simstring_measure = simstring_measure_from_string(pm.get<std::string>("simstring_measure_str"));

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

    std::string resembla_measure_all = pm["resembla_measure"];
    ResemblaEnsemble resembla_ensemble(resembla_measure_all);
    for(auto resembla_measure: split_to_resembla_measures(resembla_measure_all)){
        std::string db_path = db_path_from_resembla_measure(corpus_path, resembla_measure);
        std::string inverse_path = inverse_path_from_resembla_measure(corpus_path, resembla_measure);

        std::shared_ptr<ResemblaInterface> resembla;
        double weight = 0;
        switch(resembla_measure){
            case edit_distance:
                if((weight = pm["ed_ensemble_weight"]) == 0){
                    continue;
                }
                resembla.reset(construct_bounded_resembla(
                        db_path, inverse_path, simstring_measure, ed_simstring_threshold, ed_max_reranking_num,
                        AsIsSequenceBuilder<string_type>(),
                        EditDistance<>(STR(edit_distance))));
                break;
            case weighted_word_edit_distance:
                if((weight = pm["wwed_ensemble_weight"]) == 0){
                    continue;
                }
                resembla.reset(construct_bounded_resembla(
                        db_path, inverse_path, simstring_measure, wwed_simstring_threshold, wwed_max_reranking_num,
                        WeightedSequenceBuilder<WordSequenceBuilder, FeatureMatchWeight>(
                                WordSequenceBuilder(pm.get<std::string>("wwed_mecab_options")),
                                FeatureMatchWeight(pm.get<double>("wwed_base_weight"),
                                        pm.get<double>("wwed_delete_insert_ratio"), pm.get<double>("wwed_noun_coefficient"),
                                        pm.get<double>("wwed_verb_coefficient"), pm.get<double>("wwed_adj_coefficient"))),
                        WeightedEditDistance<SurfaceMatchCost>(STR(weighted_word_edit_distance))));
                break;
            case weighted_pronunciation_edit_distance:
                if((weight = pm["wped_ensemble_weight"]) == 0){
                    continue;
                }
                resembla.reset(construct_bounded_resembla(
                        db_path, inverse_path, simstring_measure, wped_simstring_threshold, wped_max_reranking_num,
                        PronunciationSequenceBuilder(pm.get<std::string>("wped_mecab_options"),
                                pm.get<int>("wped_mecab_feature_pos"), pm.get<std::string>("wped_mecab_pronunciation_of_marks")),
                        EditDistance<>(STR(weighted_pronunciation_edit_distance))));
                break;
            case weighted_romaji_edit_distance:
                if((weight = pm["wred_ensemble_weight"]) == 0){
                    continue;
                }
                resembla.reset(construct_bounded_resembla(
                        db_path, inverse_path, simstring_measure, wred_simstring_threshold, wred_max_reranking_num,
                        WeightedSequenceBuilder<RomajiSequenceBuilder, RomajiMatchWeight>(
                                RomajiSequenceBuilder(pm.get<std::string>("wred_mecab_options"),
                                        pm.get<int>("wred_mecab_feature_pos"), pm.get<std::string>("wred_mecab_pronunciation_of_marks")),
                                RomajiMatchWeight(pm.get<double>("wred_base_weight"), pm.get<double>("wred_delete_insert_ratio"),
                                        pm.get<double>("wred_uppercase_coefficient"), pm.get<double>("wred_lowercase_coefficient"),
                                        pm.get<double>("wred_vowel_coefficient"), pm.get<double>("wred_consonant_coefficient"))),
                        WeightedEditDistance<RomajiMatchCost>(STR(weighted_romaji_edit_distance),
                                RomajiMatchCost(pm.get<double>("wred_case_mismatch_cost"), pm.get<double>("wred_similar_letter_cost")))));
                break;
        }
        resembla_ensemble.append(resembla, weight);
    }
    return resembla_ensemble;
}

}
