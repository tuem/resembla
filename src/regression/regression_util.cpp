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

#include "regression_util.hpp"

#include <fstream>
#include <stdexcept>

#include <resembla/string_util.hpp>

#include "preprocessor/feature_extractor.hpp"
#include "preprocessor/regex_feature_extractor.hpp"

#include "aggregator/feature_aggregator.hpp"
#include "aggregator/flag_feature_aggregator.hpp"
#include "aggregator/real_feature_aggregator.hpp"

#include "predictor/svr_predictor.hpp"

#include "aggregate_regression_function.hpp"

#include "../hierarchical_resembla.hpp"

namespace resembla {

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
        if(ifs.eof() || line.length() == 0){
            break;
        }
        auto values = split(line, '\t');
        if(values.size() == 3){
            features.push_back(values);
        }
    }
    return features;
}

std::shared_ptr<ResemblaInterface> construct_regression_resembla(
        const std::shared_ptr<ResemblaInterface> resembla, paramset::manager& pm)
{
    int max_candidate = pm["svr_max_candidate"];
    std::string features_path = pm["svr_features_path"];
    std::string patterns_home = pm["svr_patterns_home"];
    std::string model_path = pm["svr_model_path"];
    std::string corpus_features_path = pm["svr_corpus_features_path"];

    auto features = load_features(features_path);
    if(features.empty()){
        throw std::runtime_error("no feature");
    }
    const auto& base_feature = features[0][0];

    std::vector<std::string> feature_names;
    auto preprocessor = std::make_shared<FeatureExtractor>();
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
            preprocessor->append(name, std::make_shared<RegexFeatureExtractor>(patterns_home + "/" + name + ".tsv"));
        }
        else{
            throw std::runtime_error("unknown feature extractor type: " + feature_extractor_type);
        }

        const auto& feature_aggregator_type = feature[2];
        if(feature_aggregator_type == "flag"){
            aggregator->append(name, std::make_shared<FlagFeatureAggregator>());
        }
        else if(feature_aggregator_type == "real"){
            aggregator->append(name, std::make_shared<RealFeatureAggregator>());
        }
        else{
            throw std::runtime_error("unknown feature aggregator type: " + feature_aggregator_type);
        }
    }

    auto predictor = std::make_shared<SVRPredictor>(feature_names, model_path);
    auto score_func = std::make_shared<AggregateRegressionFunction<FeatureAggregator, SVRPredictor>>(aggregator, predictor);

    return std::make_shared<HierarchicalResembla<FeatureExtractor,
           AggregateRegressionFunction<FeatureAggregator, SVRPredictor>>>(
                resembla, max_candidate, corpus_features_path, preprocessor, score_func);
}

}
