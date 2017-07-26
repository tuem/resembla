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

#include "feature_extractor.hpp"

#include <iostream>

#include "../../string_util.hpp"

namespace resembla {

const char FeatureExtractor::FEATURE_DELIMITER = '&';
const char FeatureExtractor::KEYVALUE_DELIMITER = '=';

FeatureExtractor::FeatureExtractor(const std::string base_similarity_key): 
    base_similarity_key(base_similarity_key){}

void FeatureExtractor::append(Feature::key_type key, std::shared_ptr<Function> func)
{
    functions[key] = func;
}

FeatureExtractor::return_type FeatureExtractor::operator()(const resembla::ResemblaResponse& input_data) const
{
#ifdef DEBUG
    std::cerr << "extract features" << std::endl;
#endif
    return_type features;
    features[base_similarity_key] = Feature::toText(input_data.score);
    for(const auto& i: functions){
        auto j = features.find(i.first);
        if(j == std::end(features)){
#ifdef DEBUG
            std::cerr << "extract feature: " << i.first << std::endl;
#endif
            features[i.first] = (*i.second)(input_data.text);
        }
        else{
#ifdef DEBUG
            std::cerr << "skip already computed feature: " << i.first << std::endl;
#endif
        }
    }
    return features;
}

FeatureExtractor::return_type FeatureExtractor::operator()(const string_type& input_text) const
{
    return_type features;
    for(const auto& i: functions){
#ifdef DEBUG
        std::cerr << "analyze feature: " << i.first << std::endl;
#endif
        features[i.first] = (*i.second)(input_text);
    }
    return features;
}

FeatureExtractor::return_type FeatureExtractor::operator()(const std::string& raw_text, const std::string& raw_features) const
{
    return_type features;
#ifdef DEBUG
    std::cerr << "load features: text=" << raw_text << ", features=" << raw_features << std::endl;
#else
    raw_text;
#endif
    for(const auto& f: split(raw_features, FEATURE_DELIMITER)){
        auto kv = split(f, KEYVALUE_DELIMITER);
        if(kv.size() == 2){
#ifdef DEBUG
            std::cerr << "load feature: " << kv[0] << "=" << kv[1] << std::endl;
#endif
            features[kv[0]] = kv[1];
        }
    }
    return features;
}

}
