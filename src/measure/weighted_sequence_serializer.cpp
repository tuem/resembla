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

#include "weighted_sequence_serializer.hpp"

#include <string>
#include <vector>

#include "../string_util.hpp"

namespace resembla {

void to_json(nlohmann::json& j, const typename WeightedSequenceBuilder<WordPreprocessor<string_type>, WordWeight>::token_type& o)
{
    std::vector<std::string> feature;
    for(const auto& f: o.token.feature){
        feature.push_back(cast_string<std::string>(f));
    }
    j = nlohmann::json{{"t", {{"s", cast_string<std::string>(o.token.surface)}, {"f", feature}}}, {"w", o.weight}};
}

void from_json(const nlohmann::json& j, typename WeightedSequenceBuilder<WordPreprocessor<string_type>, WordWeight>::token_type& o)
{
    o.token.surface = cast_string<string_type>(j.at("t").at("s").get<std::string>());
    std::vector<std::string> feature = j.at("t").at("f").get<std::vector<std::string>>();
    o.token.feature.clear();
    for(const auto& f: feature){
        o.token.feature.push_back(cast_string<string_type>(f));
    }
    o.weight = j.at("w").get<double>();
}

void to_json(nlohmann::json& j, const typename WeightedSequenceBuilder<PronunciationPreprocessor, LetterWeight<string_type>>::token_type& o)
{
    j = nlohmann::json{{"t", cast_string<std::string>(string_type(1, o.token))}, {"w", o.weight}};
}

void from_json(const nlohmann::json& j, typename WeightedSequenceBuilder<PronunciationPreprocessor, LetterWeight<string_type>>::token_type& o)
{
    o.token = cast_string<RomajiPreprocessor::output_type>(j.at("t").get<std::string>())[0];
    o.weight = j.at("w").get<double>();
}

void to_json(nlohmann::json& j, const typename WeightedSequenceBuilder<RomajiPreprocessor, RomajiWeight>::token_type& o)
{
    j = nlohmann::json{{"t", cast_string<std::string>(string_type(1, o.token))}, {"w", o.weight}};
}

void from_json(const nlohmann::json& j, typename WeightedSequenceBuilder<RomajiPreprocessor, RomajiWeight>::token_type& o)
{
    o.token = cast_string<RomajiPreprocessor::output_type>(j.at("t").get<std::string>())[0];
    o.weight = j.at("w").get<double>();
}

}
