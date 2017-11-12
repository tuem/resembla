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

#ifndef RESEMBLA_ELIMINATOR_HPP
#define RESEMBLA_ELIMINATOR_HPP

#include <string>
#include <vector>
#include <map>
#include <algorithm>

#ifdef DEBUG
#include <iostream>
#include "string_util.hpp"
#endif

namespace resembla {

template<
    typename string_type = std::string,
    typename bitvector_type = uint64_t,
    typename distance_type = long long
>
struct Eliminator
{
    using size_type = typename string_type::size_type;
    using symbol_type = typename string_type::value_type;

    Eliminator(const string_type& pattern): pattern(pattern)
    {
        if(pattern.empty()){
            return;
        }

        pattern_length = pattern.length();
        block_size = ((pattern_length - 1) >> bitOffset<bitvector_type>()) + 1;
        rest_bits = pattern_length - (block_size - 1) * bitWidth<bitvector_type>();
        sink = bitvector_type{1} << (rest_bits - 1);
        VP0 = (bitvector_type{1} << rest_bits) - 1;

        constructPM();
        zeroes.resize(block_size, 0);
        work.resize(block_size);
    }

    void operator()(std::vector<string_type>& candidates, size_type k, bool keep_tie = true)
    {
        using index_distance = std::pair<size_type, distance_type>;

        // compute scores
        std::vector<index_distance> work(candidates.size());
        for(size_type i = 0; i < work.size(); ++i){
            work[i].first = i;
            work[i].second = distance(candidates[i]);
        }

        std::nth_element(std::begin(work), std::begin(work) + k, std::end(work),
            [](const index_distance& a, const index_distance& b) -> bool{
                return a.second < b.second;
            });
        if(keep_tie){
            for(auto i = k; i < work.size(); ++i){
                if(work[k - 1].second == work[i].second){
                    std::swap(work[k++], work[i]);
                }
            }
        }

        // ensure that work[i].first < work[j].first if i < j < k
        std::sort(std::begin(work), std::begin(work) + k,
            [](const index_distance& a, const index_distance& b) -> bool{
                return a.first < b.first;
            });
#ifdef DEBUG
        std::cerr << "DEBUG: " << "eliminate " << work.size() << " strings" << std::endl;
        for(size_type i = 0; i < k; ++i){
            std::cerr << "DEBUG: " << cast_string<std::string>(candidates[work[i].first]) << ": " << work[i].second << std::endl;
        }
#endif

        // sort original list
        for(size_type i = 0; i < k; ++i){
            std::swap(candidates[i], candidates[work[i].first]);
        }
        candidates.erase(std::begin(candidates) + k, std::end(candidates));
    }

protected:
    string_type pattern;
    size_type pattern_length;
    symbol_type c_min, c_max;
    size_type block_size;
    size_type rest_bits;
    bitvector_type sink;
    bitvector_type VP0;

    std::vector<std::pair<symbol_type, std::vector<bitvector_type>>> PM;
    std::vector<bitvector_type> zeroes;

    struct WorkData
    {
        bitvector_type D0;
        bitvector_type HP;
        bitvector_type HN;
        bitvector_type VP;
        bitvector_type VN;

        void reset()
        {
            D0 = HP = HN = VN = 0;
            VP = ~(bitvector_type{0});
        }
    };
    std::vector<WorkData> work;

    template<typename Integer> static constexpr int bitWidth()
    {
        return 8 * sizeof(Integer);
    }

    static constexpr int bitOffset(int w)
    {
        return w < 2 ? 0 : (bitOffset(w >> 1) + 1);
    }

    template<typename Integer> static constexpr int bitOffset()
    {
        return bitOffset(bitWidth<Integer>());
    }

    template<typename key_type, typename value_type>
    const value_type& findValue(const std::vector<std::pair<key_type, value_type>>& data,
            const key_type c, const value_type& default_value) const
    {
        if(c < c_min || c_max < c){
            return default_value;
        }
        else if(c == c_min){
            return PM.front().second;
        }
        else if(c == c_max){
            return PM.back().second;
        }

        size_type l = 1, r = data.size() - 1;
        while(r - l > 8){
            auto i = (l + r) / 2;
            if(data[i].first < c){
                l = i + 1;
            }
            else if(data[i].first > c){
                r = i;
            }
            else{
                return data[i].second;
            }
        }

        for(size_type i = l; i < r; ++i){
            if(data[i].first == c){
                return data[i].second;
            }
        }

        return default_value;
    }

    void constructPM()
    {
        std::map<symbol_type, std::vector<bitvector_type>> PM_work;
        for(size_type i = 0; i < block_size - 1; ++i){
            for(size_type j = 0; j < bitWidth<bitvector_type>(); ++j){
                if(PM_work[pattern[i * bitWidth<bitvector_type>() + j]].empty()){
                    PM_work[pattern[i * bitWidth<bitvector_type>() + j]].resize(block_size, 0);
                }
                PM_work[pattern[i * bitWidth<bitvector_type>() + j]][i] |= bitvector_type{1} << j;
            }
        }
        for(size_type i = 0; i < rest_bits; ++i){
            if(PM_work[pattern[(block_size - 1) * bitWidth<bitvector_type>() + i]].empty()){
                PM_work[pattern[(block_size - 1) * bitWidth<bitvector_type>() + i]].resize(block_size, 0);
            }
            PM_work[pattern[(block_size - 1) * bitWidth<bitvector_type>() + i]].back() |= bitvector_type{1} << i;
        }

        PM.resize(PM_work.size());
        std::copy(std::begin(PM_work), std::end(PM_work), std::begin(PM));
        c_min = PM.front().first;
        c_max = PM.back().first;
    }

    distance_type distance_sp(const string_type& text)
    {
        auto& w = work.front();
        w.reset();
        w.VP = VP0;

        distance_type D = pattern_length;
        for(auto c: text){
            auto X = findValue(PM, c, zeroes).front() | w.VN;

            w.D0 = ((w.VP + (X & w.VP)) ^ w.VP) | X;
            w.HP = w.VN | ~(w.VP | w.D0);
            w.HN = w.VP & w.D0;

            X = (w.HP << 1) | 1;
            w.VP = (w.HN << 1) | ~(X | w.D0);
            w.VN = X & w.D0;

            if(w.HP & sink){
                ++D;
            }
            else if(w.HN & sink){
                --D;
            }
        }
        return D;
    }

    distance_type distance_lp(const string_type& text)
    {
        constexpr bitvector_type msb = bitvector_type{1} << (bitWidth<bitvector_type>() - 1);

        for(auto& w: work){
            w.reset();
        }
        work.back().VP = VP0;

        distance_type D = pattern_length;
        for(auto c: text){
            const auto& PMc = findValue(PM, c, zeroes);
            for(size_type r = 0; r < block_size; ++r){
                auto& w = work[r];
                auto X = PMc[r];
                if(r > 0 && (work[r - 1].HN & msb)){
                    X |= 1;
                }

                w.D0 = ((w.VP + (X & w.VP)) ^ w.VP) | X | w.VN;
                w.HP = w.VN | ~(w.VP | w.D0);
                w.HN = w.VP & w.D0;

                X = w.HP << 1;
                if(r == 0 || work[r - 1].HP & msb){
                    X |= 1;
                }
                w.VP = (w.HN << 1) | ~(X | w.D0);
                if(r > 0 && (work[r - 1].HN & msb)){
                    w.VP |= 1;
                }
                w.VN = X & w.D0;
            }

            if(work.back().HP & sink){
                ++D;
            }
            else if(work.back().HN & sink){
                --D;
            }
        }
        return D;
    }

    distance_type distance(const string_type& text)
    {
        if(text.empty()){
            return pattern_length;
        }
        else if(pattern_length == 0){
            return text.size();
        }

        if(block_size == 1){
            return distance_sp(text);
        }
        else{
            return distance_lp(text);
        }
    }
};

}
#endif
