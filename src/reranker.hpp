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

#ifndef RESEMBLA_RERANKER_HPP
#define RESEMBLA_RERANKER_HPP

#include <vector>
#include <algorithm>

#ifdef DEBUG
#include <string>
#include <iostream>
#include "string_util.hpp"
#endif

namespace resembla {

template<typename Original>
class Reranker
{
public:
    using output_type = std::pair<Original, double>;

    template<typename Iterator, typename ScoreFunction>
    std::vector<output_type> rerank(
        const typename std::iterator_traits<Iterator>::value_type& target,
        Iterator begin, Iterator end,
        const ScoreFunction& score_func,
        double threshold = 0.0, size_t max_output = 0
    ) const
    {
#ifdef DEBUG
        std::cerr << "DEBUG: " << "target=" << cast_string<std::string>(target.first) << std::endl;
        std::cerr << "DEBUG: " << "===========before reranking=============" << std::endl;
        for(auto i = begin; i != end; ++i){
            std::cerr << "DEBUG: " << cast_string<std::string>(i->first) << std::endl;
        }
        std::cerr << "DEBUG: " << "start reranking: threshold==" << threshold << ", max_output=" << max_output << std::endl;
#endif
        std::vector<output_type> result;
        for(auto i = begin; i != end; ++i){
            auto score = score_func(target.second, i->second);
            if(threshold == 0.0 || score >= threshold){
                result.push_back(std::make_pair(i->first, score));
            }
        }

        if(max_output != 0 && result.size() > max_output){
            std::partial_sort(std::begin(result), std::begin(result) + max_output, std::end(result), Sorter());
            result.erase(std::begin(result) + max_output, std::end(result));
        }
        else{
            std::sort(std::begin(result), std::end(result), Sorter());
        }
#ifdef DEBUG
        std::cerr << "DEBUG: " << "===========after reranking=============" << std::endl;
        for(const auto& r: result){
            std::cerr << "DEBUG: " << "text=" << cast_string<std::string>(r.first) << ", score=" << r.second << std::endl;
        }
#endif
        return result;
    }

protected:
    struct Sorter
    {
        bool operator()(const output_type& a, const output_type& b) const
        {
            return a.second > b.second;
        }
    };
};

}
#endif
