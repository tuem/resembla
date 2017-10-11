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

#include "feature_extractor.hpp"

#include "../../string_util.hpp"

#ifdef DEBUG
#include <iostream>
#endif

namespace resembla {

FeatureExtractor::FeatureExtractor(const std::string& base_similarity_key): 
    base_similarity_key(base_similarity_key)
{}

void FeatureExtractor::append(Feature::key_type key, std::shared_ptr<Function> func)
{
    functions[key] = func;
}

FeatureExtractor::output_type FeatureExtractor::operator()(const std::string& raw_text, const std::string& raw_features) const
{
#ifdef DEBUG
    std::cerr << "load features from saved data: text=" << raw_text << ", features=" << raw_features << std::endl;
#else
    (void)raw_text;
#endif
    output_type features;
    for(const auto& f: split(raw_features, attribute_delimiter<>())){
        auto kv = split(f, keyvalue_delimiter<>());
        if(kv.size() == 2){
            auto i = functions.find(kv[0]);
            if(i != std::end(functions)){
                features[kv[0]] = kv[1];
#ifdef DEBUG
                std::cerr << "load feature: key=" << kv[0] << ", value=" << kv[1] << std::endl;
#endif
            }
        }
    }
    for(const auto& i: functions){
        auto k = features.find(i.first);
        if(k == std::end(features)){
            features[i.first] = (*i.second)(cast_string<string_type>(raw_text));
#ifdef DEBUG
            std::cerr << "extract feature: key=" << i.first << ", value=" << (*i.second)(cast_string<string_type>(raw_text)) << std::endl;
#endif
        }
    }
    return features;
}

FeatureExtractor::output_type FeatureExtractor::operator()(
        const resembla::ResemblaInterface::output_type& data, const output_type& given_features) const
{
    output_type features(given_features);
    // TODO: remove this code. FeatureExtractor should accept multiple base similarities
    features[base_similarity_key] = Feature::toText(data.score);
    for(const auto& i: functions){
        auto k = features.find(i.first);
#ifdef DEBUG
        if(k == std::end(features)){
            std::cerr << "extract feature: key=" << i.first << ", value=" << (*i.second)(data.text) << std::endl;
        }
        else{
            std::cerr << "skip already computed feature: key=" << i.first << ", value=" << k->second << std::endl;
        }
#endif
        if(k == std::end(features)){
            features[i.first] = (*i.second)(data.text);
        }
    }
    return features;
}

FeatureExtractor::output_type FeatureExtractor::operator()(const string_type& text) const
{
    output_type features;
    for(const auto& i: functions){
#ifdef DEBUG
        std::cerr << "extract feature: " << i.first << std::endl;
#endif
        features[i.first] = (*i.second)(text);
    }
    return features;
}

FeatureExtractor::output_type FeatureExtractor::operator()(const string_type& text, bool is_original) const
{
    output_type features;

    auto raw_text = cast_string<std::string>(text);
    if(is_original){
        auto columns = split(raw_text, column_delimiter<>());
        if(columns.size() > 1){
            raw_text = columns[0];
            for(const auto& f: split(columns[1], attribute_delimiter<>())){
                auto kv = split(f, keyvalue_delimiter<>());
                if(kv.size() == 2){
                    auto i = functions.find(kv[0]);
                    if(i != std::end(functions)){
#ifdef DEBUG
                        std::cerr << "load feature: key=" << kv[0] << ", value=" << kv[1] << std::endl;
#endif
                        features[kv[0]] = kv[1];
                    }
                }
            }
        }
    }
    for(const auto& i: functions){
        auto k = features.find(i.first);
        if(k == std::end(features)){
            features[i.first] = (*i.second)(cast_string<string_type>(raw_text));
#ifdef DEBUG
            std::cerr << "extract feature: key=" << i.first << ", value=" << (*i.second)(cast_string<string_type>(raw_text)) << std::endl;
#endif
        }
    }
    return features;
}

string_type FeatureExtractor::index(const string_type& text) const
{
    return text;
}

}
