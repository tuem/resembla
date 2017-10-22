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

#ifndef RESEMBLA_KEYWORD_MATCHER_HPP
#define RESEMBLA_KEYWORD_MATCHER_HPP

#include "keyword_match_preprocessor.hpp"

#include <string>
#include <iostream>

namespace resembla {

template<typename StringPreprocessor>
struct KeywordMatcher
{
public:
    using string_type = typename StringPreprocessor::output_type;

    double operator()(const typename KeywordMatchPreprocessor<StringPreprocessor>::output_type& target,
            const typename KeywordMatchPreprocessor<StringPreprocessor>::output_type& reference) const
    {
        // TODO: use synonyms
        if(reference.keywords.empty()){
            return 0.0;
        }
        double score = 0.0;
        for(const auto& keyword: reference.keywords){
            // TODO: approximate match
            if(target.text.find(keyword) != string_type::npos){
                score += 1.0;
            }
            else{
                score -= 1.0;
            }
        }
#ifdef DEBUG
        for(const auto& keyword: reference.keywords){
            std::cerr << (target.text.find(keyword) == string_type::npos ? "not " : "") <<
                "matced: keyword=" << cast_string<std::string>(keyword) <<
                ", target=" << cast_string<std::string>(target.text) <<
                ", reference=" << cast_string<std::string>(reference.text) << std::endl;
        }
        std::cerr << "keyword match score=" << score / reference.keywords.size() << std::endl;
#endif
        return score / reference.keywords.size();
    }
};

}
#endif
