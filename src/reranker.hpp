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

#ifndef __RERANKER_HPP__
#define __RERANKER_HPP__

#include <iostream>
#include <vector>
#include <algorithm>

namespace resembla {

template<typename Original>
class Reranker
{
public:
    using result_type = std::pair<Original, double>;

    template<
        typename Iterator,
        typename DistanceFunction
    >
    std::vector<result_type> rerank(
        const typename std::iterator_traits<Iterator>::value_type& target,
        const Iterator begin,
        const Iterator end,
        DistanceFunction dist_func
    ) const
    {
#ifdef DEBUG
        std::wcerr << L"DEBUG: " << L"target=" << target.first << std::endl;
        std::wcerr << L"DEBUG: " << L"===========before reranking=============" << std::endl;
        for(auto i = begin; i != end; ++i){
            std::wcerr << L"DEBUG: " << i->first << std::endl;
        }
#endif
        std::vector<result_type> result;
        for(auto i = begin; i != end; ++i){
            result.push_back(std::make_pair(i->first, dist_func(i->second, target.second)));
        }
        std::sort(std::begin(result), std::end(result), Sorter());
#ifdef DEBUG
        std::wcerr << L"DEBUG: " << L"===========after reranking=============" << std::endl;
        for(auto i = std::begin(result); i != std::end(result); ++i){
            std::wcerr << L"DEBUG: " << L"text=" << i->first << L", distance=" << i->second << std::endl;
        }
#endif
        return result;
    }

protected:
    struct Sorter
    {
        bool operator()(const result_type& a, const result_type& b) const
        {
            return a.second > b.second;
        }
    };
};

}
#endif
