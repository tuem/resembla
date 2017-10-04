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

#ifndef RESEMBLA_KEYWORD_MATCH_PREPROCESSOR_HPP
#define RESEMBLA_KEYWORD_MATCH_PREPROCESSOR_HPP

#include <string>
#include <vector>
#include <iostream>

#include <json.hpp>

#include "../string_util.hpp"

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

    KeywordMatchPreprocessor()
    {
        // TODO: loadSynonyms(synonym_path);
    }

    output_type operator()(const string_type& raw_text, bool is_original = false) const
    {
        if(!is_original){
            return {raw_text, {}};
        }
        const auto key = cast_string<string_type>("keyword");
        auto columns = split(raw_text, column_delimiter<typename string_type::value_type>());
        if(columns.size() > 1){
            for(auto f: split(columns[1], feature_delimiter<typename string_type::value_type>())){
                auto kv = split(f, keyvalue_delimiter<typename string_type::value_type>());
                if(kv.size() == 2 && kv[0] == key){
#ifdef DEBUG
                    for(auto w: split(kv[1], value_delimiter<typename string_type::value_type>())){
                        std::cerr << "load keyword: text=" << cast_string<std::string>(columns[0]) << ", keyword=" << cast_string<std::string>(w) << std::endl;
                    }
#endif
                    return {columns[0], split(kv[1], value_delimiter<typename string_type::value_type>())};
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

// TODO: implement as template functions
void to_json(nlohmann::json& j, const typename KeywordMatchPreprocessor<string_type>::output_type& o);
void from_json(const nlohmann::json& j, typename KeywordMatchPreprocessor<string_type>::output_type& o);

}
#endif
