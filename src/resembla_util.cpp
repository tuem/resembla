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
#include "measure/kana_match_cost.hpp"

#include "measure/romaji_sequence_builder.hpp"
#include "measure/romaji_match_weight.hpp"
#include "measure/romaji_match_cost.hpp"

#include "measure/keyword_match_preprocessor.hpp"
#include "measure/keyword_matcher.hpp"

#include "regression/extractor/feature_extractor.hpp"
#include "regression/extractor/regex_feature_extractor.hpp"
#include "regression/extractor/date_period_feature_extractor.hpp"
#include "regression/extractor/time_period_feature_extractor.hpp"

#include "regression/aggregator/flag_feature_aggregator.hpp"
#include "regression/aggregator/real_feature_aggregator.hpp"
#include "regression/aggregator/interval_feature_aggregator.hpp"

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

std::string db_path_from_resembla_measure(const std::string& corpus_path, const measure resembla_measure)
{
    if(resembla_measure == edit_distance){
        return corpus_path + SIMSTRING_DB_FILE_COMMON_SUFFIX + STR(edit_distance);
    }
    else if(resembla_measure == weighted_word_edit_distance){
        return corpus_path + SIMSTRING_DB_FILE_COMMON_SUFFIX + STR(weighted_word_edit_distance);
    }
    else if(resembla_measure == weighted_pronunciation_edit_distance){
        return corpus_path + SIMSTRING_DB_FILE_COMMON_SUFFIX + STR(weighted_pronunciation_edit_distance);
    }
    else if(resembla_measure == weighted_romaji_edit_distance){
        return corpus_path + SIMSTRING_DB_FILE_COMMON_SUFFIX + STR(weighted_romaji_edit_distance);
    }
    else if(resembla_measure == keyword_match){
        return corpus_path + SIMSTRING_DB_FILE_COMMON_SUFFIX + STR(keyword_match);
    }
    else if(resembla_measure == svr){
        return corpus_path + SIMSTRING_DB_FILE_COMMON_SUFFIX + STR(svr);
    }
    else{
        throw std::invalid_argument("unknown Resembla measure: " + std::string(STR(resembla_measure)));
    }
}

std::string inverse_path_from_resembla_measure(const std::string& corpus_path, const measure resembla_measure)
{
    if(resembla_measure == edit_distance){
        return corpus_path + SIMSTRING_INVERSE_FILE_COMMON_SUFFIX + STR(edit_distance);
    }
    else if(resembla_measure == weighted_word_edit_distance){
        return corpus_path + SIMSTRING_INVERSE_FILE_COMMON_SUFFIX + STR(weighted_word_edit_distance);
    }
    else if(resembla_measure == weighted_pronunciation_edit_distance){
        return corpus_path + SIMSTRING_INVERSE_FILE_COMMON_SUFFIX + STR(weighted_pronunciation_edit_distance);
    }
    else if(resembla_measure == weighted_romaji_edit_distance){
        return corpus_path + SIMSTRING_INVERSE_FILE_COMMON_SUFFIX + STR(weighted_romaji_edit_distance);
    }
    else if(resembla_measure == keyword_match){
        return corpus_path + SIMSTRING_INVERSE_FILE_COMMON_SUFFIX + STR(keyword_match);
    }
    else if(resembla_measure == svr){
        return corpus_path + SIMSTRING_INVERSE_FILE_COMMON_SUFFIX + STR(svr);
    }
    else{
        throw std::invalid_argument("unknown Resembla measure: " + std::string(STR(resembla_measure)));
    }
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
        else if(resembla_measure_str == STR(keyword_match)){
            result.push_back(keyword_match);
        }
        else if(resembla_measure_str == STR(svr)){
            result.push_back(svr);
        }
        else{
            if(!ignore_unknown_measure){
                throw std::invalid_argument("unknown Resembla measure: " + resembla_measure_str);
            }
        }
    }
    return result;
}

std::shared_ptr<ResemblaRegression<Composition<FeatureAggregator, SVRPredictor>>> construct_resembla_regression(
        int max_candidate, std::string inverse_path,
        std::string features_path, std::string patterns_home, std::string model_path,
        const std::shared_ptr<ResemblaInterface> resembla)
{
    auto features = load_features(features_path);
    if(features.empty()){
        throw std::runtime_error("no feature");
    }
    const auto& base_feature = features[0][0];

    std::vector<std::string> feature_names;
    auto extractor = std::make_shared<FeatureExtractor>();
    auto aggregator= std::make_shared<FeatureAggregator>();
    for(const auto& feature: features){
        const auto& name = feature[0];
        feature_names.push_back(name);
        if(name == base_feature){
            aggregator->append(name, nullptr);
            continue;
        }

        const auto& feature_extractor_type = feature[1];
        if(feature_extractor_type == "re"){
            extractor->append(name, std::make_shared<RegexFeatureExtractor>(patterns_home + "/" + name + ".tsv"));
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

        const auto& feature_aggregator_type = feature[2];
        if(feature_aggregator_type == "flag"){
            aggregator->append(name, std::make_shared<FlagFeatureAggregator>());
        }
        else if(feature_aggregator_type == "real"){
            aggregator->append(name, std::make_shared<RealFeatureAggregator>());
        }
        else if(feature_aggregator_type == "interval"){
            aggregator->append(name, std::make_shared<IntervalFeatureAggregator>());
        }
        else if(feature_aggregator_type == "-"){
            aggregator->append(name, nullptr);
        }
        else{
            throw std::runtime_error("unknown feature aggregator type: " + feature_aggregator_type);
        }
    }

    // aggregated features => score
    auto original_predictor = std::make_shared<SVRPredictor>(feature_names, model_path);
    // pair of features => score
    auto predictor = std::make_shared<Composition<FeatureAggregator, SVRPredictor>>(aggregator, original_predictor);

    auto resembla_regression = std::make_shared<
            ResemblaRegression<Composition<FeatureAggregator, SVRPredictor>>>(
                max_candidate, extractor, predictor, inverse_path);
    resembla_regression->append("base_similarity", resembla, true);
    return resembla_regression;
}

std::shared_ptr<ResemblaInterface> construct_resembla(std::string corpus_path, paramset::manager& pm)
{
    int simstring_measure = simstring_measure_from_string(pm.get<std::string>("simstring_measure_str"));
    std::string resembla_measure_all = pm["resembla_measure"];

    std::vector<std::pair<std::shared_ptr<ResemblaInterface>, double>> basic_resemblas;
    std::shared_ptr<ResemblaInterface> keyword_resembla = nullptr;
    bool use_regression = false;
    for(auto resembla_measure: split_to_resembla_measures(resembla_measure_all)){
        auto db_path = db_path_from_resembla_measure(corpus_path, resembla_measure);
        auto inverse_path = inverse_path_from_resembla_measure(corpus_path, resembla_measure);

        std::shared_ptr<ResemblaInterface> new_resembla;
        switch(resembla_measure){
            case svr:
                use_regression = true;
                break;
            case edit_distance:
                basic_resemblas.push_back(std::make_pair(
                    construct_basic_resembla(
                        db_path, inverse_path, simstring_measure,
                        pm.get<double>("ed_simstring_threshold") != -1 ?
                            pm.get<double>("ed_simstring_threshold") : pm.get<double>("simstring_threshold"),
                        pm.get<int>("ed_max_reranking_num") != -1 ?
                            pm.get<int>("ed_max_reranking_num") : pm.get<int>("resembla_max_reranking_num"),
                        std::make_shared<AsIsSequenceBuilder<string_type>>(),
                        std::make_shared<EditDistance<>>(STR(edit_distance)), true),
                    pm.get<double>("ed_ensemble_weight")));
                break;
            case weighted_word_edit_distance:
                basic_resemblas.push_back(std::make_pair(
                    construct_basic_resembla(
                        db_path, inverse_path, simstring_measure,
                            pm.get<double>("wwed_simstring_threshold") != -1 ?
                            pm.get<double>("wwed_simstring_threshold") : pm.get<double>("simstring_threshold"),
                        pm.get<int>("wwed_max_reranking_num") != -1 ?
                            pm.get<int>("wwed_max_reranking_num") : pm.get<int>("resembla_max_reranking_num"),
                        std::make_shared<WeightedSequenceBuilder<WordSequenceBuilder, FeatureMatchWeight>>(
                            WordSequenceBuilder(pm.get<std::string>("wwed_mecab_options")),
                            FeatureMatchWeight(pm.get<double>("wwed_base_weight"),
                                pm.get<double>("wwed_delete_insert_ratio"), pm.get<double>("wwed_noun_coefficient"),
                                pm.get<double>("wwed_verb_coefficient"), pm.get<double>("wwed_adj_coefficient"))),
                        std::make_shared<WeightedEditDistance<SurfaceMatchCost>>(STR(weighted_word_edit_distance)), true),
                    pm.get<double>("wwed_ensemble_weight")));
                break;
            case weighted_pronunciation_edit_distance:
                basic_resemblas.push_back(std::make_pair(
                    construct_basic_resembla(
                        db_path, inverse_path, simstring_measure,
                        pm.get<double>("wped_simstring_threshold") != -1 ?
                            pm.get<double>("wped_simstring_threshold") : pm.get<double>("simstring_threshold"),
                        pm.get<int>("wped_max_reranking_num") != -1 ?
                            pm.get<int>("wped_max_reranking_num") : pm.get<int>("resembla_max_reranking_num"),
                        std::make_shared<PronunciationSequenceBuilder>(pm.get<std::string>("wped_mecab_options"),
                            pm.get<int>("wped_mecab_feature_pos"), pm.get<std::string>("wped_mecab_pronunciation_of_marks")),
                        std::make_shared<EditDistance<KanaMatchCost<string_type>>>(
                            STR(weighted_pronunciation_edit_distance), pm.get<std::string>("wped_mismatch_cost_path")),
                        true),
                    pm.get<double>("wped_ensemble_weight")));
                break;
            case weighted_romaji_edit_distance:
                basic_resemblas.push_back(std::make_pair(
                    construct_basic_resembla(
                        db_path, inverse_path, simstring_measure,
                        pm.get<double>("wred_simstring_threshold") != -1 ?
                            pm.get<double>("wred_simstring_threshold") : pm.get<double>("simstring_threshold"),
                        pm.get<int>("wred_max_reranking_num") != -1 ?
                            pm.get<int>("wred_max_reranking_num") : pm.get<int>("resembla_max_reranking_num"),
                        std::make_shared<WeightedSequenceBuilder<RomajiSequenceBuilder, RomajiMatchWeight>>(
                            RomajiSequenceBuilder(pm.get<std::string>("wred_mecab_options"),
                                pm.get<int>("wred_mecab_feature_pos"), pm.get<std::string>("wred_mecab_pronunciation_of_marks")),
                            RomajiMatchWeight(pm.get<double>("wred_base_weight"), pm.get<double>("wred_delete_insert_ratio"),
                                pm.get<double>("wred_uppercase_coefficient"), pm.get<double>("wred_lowercase_coefficient"),
                                pm.get<double>("wred_vowel_coefficient"), pm.get<double>("wred_consonant_coefficient"))),
                        pm.get<std::string>("wred_mismatch_cost_path").empty() ?
                            std::make_shared<WeightedEditDistance<RomajiMatchCost>>(STR(weighted_romaji_edit_distance),
                                RomajiMatchCost(pm.get<double>("wred_case_mismatch_cost"), pm.get<double>("wred_similar_letter_cost"))) :
                            std::make_shared<WeightedEditDistance<RomajiMatchCost>>(STR(weighted_romaji_edit_distance),
                                RomajiMatchCost(pm.get<std::string>("wred_mismatch_cost_path"),
                                pm.get<double>("wred_case_mismatch_cost"))),
                        true),
                    pm.get<double>("wred_ensemble_weight")));
                break;
            case keyword_match:
                keyword_resembla = construct_basic_resembla(
                    db_path, inverse_path, simstring_measure,
                    pm.get<double>("km_simstring_threshold") != -1 ?
                        pm.get<double>("km_simstring_threshold") : pm.get<double>("simstring_threshold"),
                    pm.get<int>("km_max_reranking_num") != -1 ?
                        pm.get<int>("km_max_reranking_num") : pm.get<int>("resembla_max_reranking_num"),
                    std::make_shared<KeywordMatchPreprocessor<string_type>>(),
                    std::make_shared<KeywordMatcher<string_type>>(STR(keyword_match)), true);
                break;
        }
    }
    if(basic_resemblas.size() == 0 && keyword_resembla == nullptr){
        throw std::runtime_error("at least one BasicResembla instance is needed: " + resembla_measure_all);
    }

    if(!use_regression && keyword_resembla != nullptr){
        basic_resemblas.push_back(std::make_pair(keyword_resembla, pm.get<double>("km_ensemble_weight")));
        keyword_resembla = nullptr;
    }

    std::shared_ptr<ResemblaInterface> base_resembla;
    if(basic_resemblas.size() > 0){
        if(basic_resemblas.size() == 1){
            base_resembla = basic_resemblas[0].first;
        }
        else{
            std::shared_ptr<ResemblaEnsemble> resembla_ensemble =
                std::make_shared<ResemblaEnsemble>(resembla_measure_all, pm.get<double>("resembla_max_reranking_num"));
            for(auto p: basic_resemblas){
                if(p.second > 0){
                    resembla_ensemble->append(p.first, p.second);
                }
            }
            base_resembla = resembla_ensemble;
        }
    }
    else{
        base_resembla = keyword_resembla;
    }

    std::shared_ptr<ResemblaInterface> resembla;
    if(use_regression){
        std::shared_ptr<ResemblaRegression<Composition<FeatureAggregator, SVRPredictor>>>
            resembla_regression = construct_resembla_regression(pm.get<int>("svr_max_candidate"),
                inverse_path_from_resembla_measure(corpus_path, svr),
                pm.get<std::string>("svr_features_path"), pm.get<std::string>("svr_patterns_home"),
                pm.get<std::string>("svr_model_path"), base_resembla);
        if(base_resembla != keyword_resembla){
            resembla_regression->append(STR(keyword_match), keyword_resembla, false);
        }
        resembla = resembla_regression;
    }
    else{
        resembla = base_resembla;
    }

    return resembla;
}

std::vector<std::vector<std::string>> load_features(const std::string file_path)
{
    std::ifstream ifs(file_path);
    if(ifs.fail()){
        throw std::runtime_error("input file is not available: " + file_path);
    }

    std::vector<std::vector<std::string>> features;
    while(ifs.good()){
        std::string line;
        std::getline(ifs, line);
        if(ifs.eof()){
            break;
        }
        else if(line.compare(0, 1, "#") == 0 || line.empty()){
            continue;
        }

        auto values = split(line, column_delimiter<>());
        if(values.size() == 3){
            features.push_back(values);
        }
    }
    return features;
}

}
