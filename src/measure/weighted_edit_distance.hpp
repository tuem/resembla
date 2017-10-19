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

#ifndef RESEMBLA_WEIGHTED_EDIT_DISTANCE_HPP
#define RESEMBLA_WEIGHTED_EDIT_DISTANCE_HPP

#include <string>
#include <vector>

#include "fixed_cost.hpp"

namespace resembla {

template<typename CostFunction = FixedCost>
struct WeightedEditDistance
{
    CostFunction cost;

    WeightedEditDistance(CostFunction cost = CostFunction()): cost(cost) {}

    template<typename sequence_type>
    double operator()(const sequence_type& a, const sequence_type& b) const
    {
        if(a.empty()){
            return b.empty() ? 1.0 : 0.0;
        }
        else if(b.empty()){
            return 0.0;
        }

        // prepare work table
        std::vector<double> D(a.size() + 1);
        D[0] = 0;
        for(size_t i = 1; i < a.size() + 1; ++i){
            D[i] = D[i - 1] + a[i - 1].weight;
        }

        // compute edit distance
        auto max_cost = D.back();
        for(const auto& c: b){
            auto prev = D[0];
            D[0] += c.weight;
            for(size_t i = 1; i < a.size() + 1; ++i){
                auto del = D[i - 1] + a[i - 1].weight;
                auto ins = D[i] + c.weight;
                auto sub = prev + (a[i - 1].weight + c.weight) * cost(a[i - 1].token, c.token);
                prev = D[i];
                D[i] = std::min({del, ins, sub});
            }
        }
        max_cost += D.front();
    
        return 1.0 - D.back() / max_cost;
    }
};

}
#endif
