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

#include "resembla_util.hpp"

#include <simstring/simstring.h>

#include "simstring_database.hpp"

#include "measure/asis_preprocessor.hpp"

#include "measure/word_preprocessor.hpp"
#include "measure/word_weight.hpp"
#include "measure/word_mismatch_cost.hpp"

#include "measure/pronunciation_preprocessor.hpp"
#include "measure/letter_weight.hpp"
#include "measure/kana_mismatch_cost.hpp"

#include "measure/romaji_preprocessor.hpp"
#include "measure/romaji_weight.hpp"
#include "measure/romaji_mismatch_cost.hpp"

#include "measure/keyword_match_preprocessor.hpp"
#include "measure/keyword_matcher.hpp"

#include "measure/weighted_sequence_builder.hpp"
#include "measure/weighted_sequence_serializer.hpp"

#include "measure/edit_distance.hpp"
#include "measure/weighted_edit_distance.hpp"

#include "measure/weighted_l2_norm.hpp"
#include "resembla_ensemble.hpp"

#include "regression/extractor/feature_extractor.hpp"
#include "regression/extractor/regex_feature_extractor.hpp"
#include "regression/extractor/date_period_feature_extractor.hpp"
#include "regression/extractor/time_period_feature_extractor.hpp"

#include "regression/aggregator/flag_feature_aggregator.hpp"
#include "regression/aggregator/real_feature_aggregator.hpp"
#include "regression/aggregator/interval_feature_aggregator.hpp"

namespace resembla {

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
    else if(resembla_measure == ensemble){
        return corpus_path + SIMSTRING_DB_FILE_COMMON_SUFFIX + STR(ensemble);
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
    else if(resembla_measure == ensemble){
        return corpus_path + SIMSTRING_INVERSE_FILE_COMMON_SUFFIX + STR(ensemble);
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

std::shared_ptr<ResemblaRegression<SimStringDatabase<RomajiPreprocessor>, Composition<FeatureAggregator, SVRPredictor>>>
construct_resembla_regression(const std::string& simstring_db_path, const std::string& resembla_index_path,
        const paramset::manager& pm, const std::shared_ptr<ResemblaInterface> resembla)
{
    auto indexer = std::make_shared<RomajiPreprocessor>(pm.get<std::string>("index_romaji_mecab_options"),
            pm.get<int>("index_romaji_mecab_feature_pos"),
            pm.get<std::string>("index_romaji_mecab_pronunciation_of_marks"));
    auto database = std::make_shared<SimStringDatabase<RomajiPreprocessor>>(simstring_db_path,
            pm.get<int>("simstring_measure"), pm.get<double>("ed_simstring_threshold"),
            indexer, resembla_index_path);

    auto features = load_features(pm.get<std::string>("svr_features_path"));
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
    auto original_predictor = std::make_shared<SVRPredictor>(feature_names, pm.get<std::string>("svr_model_path"));
    // pair of features => score
    auto predictor = std::make_shared<Composition<FeatureAggregator, SVRPredictor>>(aggregator, original_predictor);

    auto resembla_regression = std::make_shared<
            ResemblaRegression<SimStringDatabase<RomajiPreprocessor>, Composition<FeatureAggregator, SVRPredictor>>>(
                database, extractor, predictor, pm.get<int>("svr_max_candidate"), resembla_index_path);
    resembla_regression->append("base_similarity", resembla);
    return resembla_regression;
}

std::shared_ptr<ResemblaInterface> construct_resembla(const paramset::manager& pm)
{
    auto corpus_path = pm.get<std::string>("corpus_path");
    auto resembla_measure_all = pm.get<std::string>("resembla_measure");

    std::vector<std::pair<std::shared_ptr<ResemblaInterface>, double>> basic_resemblas;
    std::shared_ptr<ResemblaInterface> keyword_resembla = nullptr;
    bool use_regression = false;
    for(auto resembla_measure: split_to_resembla_measures(resembla_measure_all)){
        auto simstring_db_path = db_path_from_resembla_measure(corpus_path, resembla_measure);
        auto resembla_index_path = inverse_path_from_resembla_measure(corpus_path, resembla_measure);

        std::shared_ptr<WordPreprocessor<string_type>> word_preprocessor;
        std::shared_ptr<PronunciationPreprocessor> pronunciation_preprocessor;
        std::shared_ptr<RomajiPreprocessor> romaji_preprocessor;

        std::shared_ptr<ResemblaInterface> new_resembla;
        switch(resembla_measure){
            case svr:
                use_regression = true;
                break;
            case edit_distance:
                basic_resemblas.push_back(std::make_pair(
                    construct_basic_resembla(
                        std::make_shared<SimStringDatabase<AsIsPreprocessor<string_type>>>(simstring_db_path,
                            pm.get<int>("simstring_measure"), pm.get<double>("ed_simstring_threshold"),
                            std::make_shared<AsIsPreprocessor<string_type>>(), resembla_index_path),
                        std::make_shared<AsIsPreprocessor<string_type>>(),
                        std::make_shared<EditDistance<>>(),
                        pm.get<int>("ed_max_reranking_num"), resembla_index_path),
                    pm.get<double>("ed_ensemble_weight")));
                break;
            case weighted_word_edit_distance:
                word_preprocessor = std::make_shared<WordPreprocessor<string_type>>(pm.get<std::string>("wwed_mecab_options"));
                basic_resemblas.push_back(std::make_pair(
                    construct_basic_resembla(
                        std::make_shared<SimStringDatabase<AsIsPreprocessor<string_type>>>(simstring_db_path,
                            pm.get<int>("simstring_measure"), pm.get<double>("wwed_simstring_threshold"),
                            std::make_shared<AsIsPreprocessor<string_type>>(), resembla_index_path),
                        std::make_shared<WeightedSequenceBuilder<WordPreprocessor<string_type>, WordWeight>>(
                            word_preprocessor, 
                            std::make_shared<WordWeight>(pm.get<double>("wwed_base_weight"),
                                pm.get<double>("wwed_delete_insert_ratio"), pm.get<double>("wwed_noun_coefficient"),
                                pm.get<double>("wwed_verb_coefficient"), pm.get<double>("wwed_adj_coefficient"))),
                        std::make_shared<WeightedEditDistance<WordMismatchCost<string_type>>>(),
                        pm.get<int>("wwed_max_reranking_num"), resembla_index_path),
                    pm.get<double>("wwed_ensemble_weight")));
                break;
            case weighted_pronunciation_edit_distance:
                pronunciation_preprocessor = std::make_shared<PronunciationPreprocessor>(pm.get<std::string>("wped_mecab_options"),
                    pm.get<int>("wped_mecab_feature_pos"), pm.get<std::string>("wped_mecab_pronunciation_of_marks"));
                basic_resemblas.push_back(std::make_pair(
                    construct_basic_resembla(
                        std::make_shared<SimStringDatabase<PronunciationPreprocessor>>(simstring_db_path,
                            pm.get<int>("simstring_measure"), pm.get<double>("wped_simstring_threshold"),
                            pronunciation_preprocessor, resembla_index_path),
                        std::make_shared<WeightedSequenceBuilder<PronunciationPreprocessor, LetterWeight<string_type>>>(
                            pronunciation_preprocessor, 
                            std::make_shared<LetterWeight<string_type>>(pm.get<double>("wped_base_weight"),
                                pm.get<double>("wped_delete_insert_ratio"), pm.get<std::string>("wped_letter_weight_path"))),
                        std::make_shared<WeightedEditDistance<KanaMismatchCost<string_type>>>(
                            pm.get<std::string>("wped_mismatch_cost_path")),
                        pm.get<int>("wped_max_reranking_num"), resembla_index_path),
                    pm.get<double>("wped_ensemble_weight")));
                break;
            case weighted_romaji_edit_distance:
                romaji_preprocessor = std::make_shared<RomajiPreprocessor>(pm.get<std::string>("wred_mecab_options"),
                    pm.get<int>("wred_mecab_feature_pos"), pm.get<std::string>("wred_mecab_pronunciation_of_marks"));
                basic_resemblas.push_back(std::make_pair(
                    construct_basic_resembla(
                        std::make_shared<SimStringDatabase<RomajiPreprocessor>>(simstring_db_path,
                            pm.get<int>("simstring_measure"), pm.get<double>("wred_simstring_threshold"),
                            romaji_preprocessor, resembla_index_path),
                        std::make_shared<WeightedSequenceBuilder<RomajiPreprocessor, RomajiWeight>>(
                            romaji_preprocessor, 
                            std::make_shared<RomajiWeight>(
                                pm.get<double>("wred_base_weight"), pm.get<double>("wred_delete_insert_ratio"),
                                pm.get<double>("wred_uppercase_coefficient"), pm.get<double>("wred_lowercase_coefficient"),
                                pm.get<double>("wred_vowel_coefficient"), pm.get<double>("wred_consonant_coefficient"))),
                        std::make_shared<WeightedEditDistance<RomajiMismatchCost>>(
                            RomajiMismatchCost(pm.get<std::string>("wred_mismatch_cost_path"),
                                pm.get<double>("wred_case_mismatch_cost"))),
                        pm.get<int>("wred_max_reranking_num"), resembla_index_path),
                    pm.get<double>("wred_ensemble_weight")));
                break;
            case keyword_match:
                keyword_resembla = construct_basic_resembla(
                    std::make_shared<SimStringDatabase<AsIsPreprocessor<string_type>>>(simstring_db_path,
                        pm.get<int>("simstring_measure"), pm.get<double>("km_simstring_threshold"),
                        std::make_shared<AsIsPreprocessor<string_type>>(), resembla_index_path),
                    std::make_shared<KeywordMatchPreprocessor<RomajiPreprocessor>>(
                        std::make_shared<RomajiPreprocessor>(pm.get<std::string>("index_romaji_mecab_options"),
                            pm.get<int>("index_romaji_mecab_feature_pos"),
                            pm.get<std::string>("index_romaji_mecab_pronunciation_of_marks"))),
                    std::make_shared<KeywordMatcher<RomajiPreprocessor>>(),
                    pm.get<int>("km_max_reranking_num"), resembla_index_path);
                break;
            case ensemble:
                break;
        }
    }
    if(basic_resemblas.empty() && keyword_resembla == nullptr){
        throw std::runtime_error("at least one BasicResembla instance is needed: " + resembla_measure_all);
    }

    if(!use_regression && keyword_resembla != nullptr){
        basic_resemblas.push_back(std::make_pair(keyword_resembla, pm.get<double>("km_ensemble_weight")));
        keyword_resembla = nullptr;
    }

    std::shared_ptr<ResemblaInterface> base_resembla;
    if(!basic_resemblas.empty()){
        if(basic_resemblas.size() == 1){
            base_resembla = basic_resemblas[0].first;
        }
        else{
            auto indexer = std::make_shared<RomajiPreprocessor>((pm.get<std::string>("index_romaji_mecab_options"),
                    pm.get<int>("index_romaji_mecab_feature_pos"), pm.get<std::string>("index_romaji_mecab_pronunciation_of_marks")));
            auto simstring_db_path = db_path_from_resembla_measure(corpus_path, ensemble);
            auto resembla_index_path = inverse_path_from_resembla_measure(corpus_path, ensemble);

            auto resembla_ensemble = std::make_shared<ResemblaEnsemble<SimStringDatabase<RomajiPreprocessor>, WeightedL2Norm<>>>(
                std::make_shared<SimStringDatabase<RomajiPreprocessor>>(simstring_db_path,
                    pm.get<int>("simstring_measure"), pm.get<double>("wred_simstring_threshold"),
                    std::make_shared<RomajiPreprocessor>(
                        pm.get<std::string>("wred_mecab_options"), pm.get<int>("wred_mecab_feature_pos"),
                        pm.get<std::string>("wred_mecab_pronunciation_of_marks")), resembla_index_path),
                std::make_shared<WeightedL2Norm<>>(), pm.get<double>("ensemble_max_candidate"));

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
        std::shared_ptr<ResemblaRegression<SimStringDatabase<RomajiPreprocessor>, Composition<FeatureAggregator, SVRPredictor>>>
            resembla_regression = construct_resembla_regression(
                db_path_from_resembla_measure(corpus_path, svr),
                inverse_path_from_resembla_measure(corpus_path, svr),
                pm, base_resembla);
        if(keyword_resembla != nullptr && base_resembla != keyword_resembla){
            resembla_regression->append(STR(keyword_match), keyword_resembla);
        }
        resembla = resembla_regression;
    }
    else{
        resembla = base_resembla;
    }

    return resembla;
}

std::vector<std::vector<std::string>> load_features(const std::string& file_path)
{
    std::vector<std::vector<std::string>> features;
    for(const auto& columns: CsvReader<>(file_path, 3)){
        features.push_back(columns);
    }
    return features;
}

}
