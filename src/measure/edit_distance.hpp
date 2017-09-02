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

#ifndef RESEMBLA_EDIT_DISTANCE_HPP
#define RESEMBLA_EDIT_DISTANCE_HPP

#include <string>
#include <vector>

#include "uniform_cost.hpp"

namespace resembla {

template<typename CostFunction = UniformCost>
struct EditDistance
{
    const std::string name;
    CostFunction cost_func;

    EditDistance(const std::string name = "edit", CostFunction cost_func = CostFunction()): name(name), cost_func(cost_func) {}

    template<typename sequence_type>
    double operator()(const sequence_type& a, const sequence_type& b) const
    {
        // prepare work table
        std::vector<std::vector<double>> D(a.size() + 1, std::vector<double>(b.size() + 1));
        D[0][0] = 0;
        for(size_t i = 1; i < a.size() + 1; ++i){
            D[i][0] = D[i - 1][0] + 1.0;
        }
        for(size_t j = 1; j < b.size() + 1; ++j){
            D[0][j] = D[0][j - 1] + 1.0;
        }
        double total_cost = D[a.size()][0] + D[0][b.size()];

        // compute edit distance
        for(size_t i = 1; i < a.size() + 1; ++i){
            for(size_t j = 1; j < b.size() + 1; ++j){
                double d_delete = D[i - 1][j] + 1;
                double d_insert = D[i][j - 1] + 1;
                double d_replace = D[i - 1][j - 1] + 2.0 * cost_func(a[i - 1], b[j - 1]);
                D[i][j] = std::min({d_delete, d_insert, d_replace});
            }
        }
    
        return 1.0 - D[a.size()][b.size()] / total_cost;
    }
};

}
#endif
