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
#include <memory>

#include <json.hpp>

#include "../string_util.hpp"
#include "asis_preprocessor.hpp"
#include "romaji_preprocessor.hpp"

#ifdef DEBUG
#include <iostream>
#endif

namespace resembla {

template<typename StringPreprocessor>
class KeywordMatchPreprocessor
{
public:
    using string_type = typename StringPreprocessor::output_type;

    struct output_type
    {
        string_type text;
        std::vector<string_type> keywords;
    };

    KeywordMatchPreprocessor(std::shared_ptr<StringPreprocessor> preprocess): preprocess(preprocess)
    {
        // TODO: loadSynonyms(synonym_path);
    }

    output_type operator()(const string_type& raw_text, bool is_original = false) const
    {
        if(!is_original){
            return {(*preprocess)(raw_text), {}};
        }

        const auto key = cast_string<string_type>("keyword");
        auto columns = split(raw_text, column_delimiter<typename string_type::value_type>());
        if(columns.size() > 1){
            auto preprocessed_text = (*preprocess)(columns[0]);
            for(auto f: split(columns[1], attribute_delimiter<typename string_type::value_type>())){
                auto kv = split(f, keyvalue_delimiter<typename string_type::value_type>());
                if(kv.size() == 2 && kv[0] == key){
#ifdef DEBUG
                    for(auto w: split(kv[1], value_delimiter<typename string_type::value_type>())){
                        std::cerr << "load keyword: text=" << cast_string<std::string>((*preprocess)(columns[0])) <<
                            ", keyword=" << cast_string<std::string>((*preprocess)(w)) << std::endl;
                    }
#endif
                    std::vector<string_type> keywords;
                    for(auto w: split(kv[1], value_delimiter<typename string_type::value_type>())){
                        keywords.push_back((*preprocess)(w));
                    }
                    return {preprocessed_text, keywords};
                }
            }
            return {preprocessed_text, {}};
        }
        return {(*preprocess)(raw_text), {}};
    }

protected:
    std::shared_ptr<StringPreprocessor> preprocess;

// TODO: use synonym dictionary to improve keyword matching quality
//    std::vector<std::vector<string_type>> synonyms;
//    std::unordered_map<string_type, size_t> synonym_index;
};

// TODO: implement as template functions
void to_json(nlohmann::json& j,
        const typename KeywordMatchPreprocessor<AsIsPreprocessor<string_type>>::output_type& o);
void from_json(const nlohmann::json& j,
        typename KeywordMatchPreprocessor<AsIsPreprocessor<string_type>>::output_type& o);
void to_json(nlohmann::json& j,
        const typename KeywordMatchPreprocessor<RomajiPreprocessor>::output_type& o);
void from_json(const nlohmann::json& j,
        typename KeywordMatchPreprocessor<RomajiPreprocessor>::output_type& o);

}
#endif
