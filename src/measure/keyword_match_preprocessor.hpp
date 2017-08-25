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

#ifndef __KEYWORD_MATCH_PREPROCESSOR_HPP__
#define __KEYWORD_MATCH_PREPROCESSOR_HPP__

#include <string>
#include <vector>
#include <iostream>

#include <json.hpp>

namespace resembla {

template<typename string_type>
class KeywordMatchPreprocessor
{
public:
    struct output_type
    {
        string_type text;
        std::vector<string_type> keywords;
    };
    using output_type = output_type;

    KeywordMatchPreprocessor()
    {
        // TODO: loadSynonyms(synonym_path);
    }

    output_type operator()(const string_type& raw_text, bool is_original = false) const
    {
        if(!is_original){
            return {raw_text, {}};
        }
        auto columns = split(raw_text, L'\t');//TODO
        if(columns.size() > 1){
            for(auto f: split(columns[1], L'&')){
                auto kv = split(f, L'=');
                if(kv.size() == 2 && kv[0] == L"keyword"){
#ifdef DEBUG
                    for(auto w: split(kv[1], L',')){
                        std::cerr << "load keyword: text=" << cast_string<std::string>(columns[0]) << ", keyword=" << cast_string<std::string>(w) << std::endl;
                    }
#endif
                    return {columns[0], split(kv[1], L',')};
                }
            }
            return {columns[0], {}};
        }
        return {raw_text, {}};
    }

    string_type index(const string_type& text) const
    {
        return text;
    }

protected:
// TODO: use synonym dictionary to improve keyword matching quality
//    std::vector<std::vector<string_type>> synonyms;
//    std::unordered_map<string_type, size_t> synonym_index;
};

void to_json(nlohmann::json& j, const typename KeywordMatchPreprocessor<string_type>::output_type& o) {
    std::vector<std::string> keywords;
    for(const auto& k: o.keywords){
        keywords.push_back(cast_string<std::string>(k));
    }
    j = nlohmann::json{{"text", cast_string<std::string>(o.text)}, {"keywords", keywords}};
}

void from_json(const nlohmann::json& j, typename KeywordMatchPreprocessor<string_type>::output_type& o) {
    o.text = cast_string<string_type>(j.at("text").get<std::string>());
    std::vector<std::string> keywords = j.at("keywords").get<std::vector<std::string>>();
    o.keywords.clear();
    for(const auto& k: keywords){
        o.keywords.push_back(cast_string<string_type>(k));
    }
}

}
#endif
