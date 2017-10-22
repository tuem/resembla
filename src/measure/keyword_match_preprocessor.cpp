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

#include "keyword_match_preprocessor.hpp"

namespace resembla {

void to_json(nlohmann::json& j,
        const typename KeywordMatchPreprocessor<AsIsPreprocessor<string_type>>::output_type& o)
{
    std::vector<std::string> keywords;
    for(const auto& k: o.keywords){
        keywords.push_back(cast_string<std::string>(k));
    }
    j = nlohmann::json{{"t", cast_string<std::string>(o.text)}, {"k", keywords}};
}

void from_json(const nlohmann::json& j,
        typename KeywordMatchPreprocessor<AsIsPreprocessor<string_type>>::output_type& o)
{
    o.text = cast_string<string_type>(j.at("t").get<std::string>());
    std::vector<std::string> keywords = j.at("k").get<std::vector<std::string>>();
    o.keywords.clear();
    for(const auto& k: keywords){
        o.keywords.push_back(cast_string<string_type>(k));
    }
}

void to_json(nlohmann::json& j,
        const typename KeywordMatchPreprocessor<RomajiPreprocessor>::output_type& o)
{
    std::vector<std::string> keywords;
    for(const auto& k: o.keywords){
        keywords.push_back(cast_string<std::string>(k));
    }
    j = nlohmann::json{{"t", cast_string<std::string>(o.text)}, {"k", keywords}};
}

void from_json(const nlohmann::json& j,
        typename KeywordMatchPreprocessor<RomajiPreprocessor>::output_type& o)
{
    o.text = cast_string<string_type>(j.at("t").get<std::string>());
    std::vector<std::string> keywords = j.at("k").get<std::vector<std::string>>();
    o.keywords.clear();
    for(const auto& k: keywords){
        o.keywords.push_back(cast_string<string_type>(k));
    }
}

}
