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

#ifndef RESEMBLA_EDIT_DISTANCE_HPP
#define RESEMBLA_EDIT_DISTANCE_HPP

#include <string>
#include <vector>

#include "fixed_cost.hpp"

namespace resembla {

template<typename CostFunction = FixedCost>
struct EditDistance
{
    const std::string name;
    CostFunction cost_func;

    EditDistance(const std::string name = "edit", CostFunction cost_func = CostFunction()):
        name(name), cost_func(cost_func) {}

    template<typename sequence_type>
    double operator()(const sequence_type& a, const sequence_type& b) const
    {
        // prepare work table
        std::vector<double> D(a.size() + 1);
        D[0] = 0;
        for(size_t i = 1; i < a.size() + 1; ++i){
            D[i] = D[i - 1] + 1.0;
        }

        for(const auto c: b){
            double prev = D[0];
            D[0] += 1.0;
            for(size_t i = 1; i < a.size() + 1; ++i){
                auto del = D[i - 1] + 1.0;
                auto ins = D[i] + 1.0;
                auto rep = prev + 2.0 * cost_func(a[i - 1], c);
                prev = D[i];
                D[i] = std::min({del, ins, rep});
            }
        }
    
        return 1.0 - D.back() / (a.size() + b.size());
    }
};

}
#endif
