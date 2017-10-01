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

#ifndef RESEMBLA_WEIGHTED_L2_NORM_HPP
#define RESEMBLA_WEIGHTED_L2_NORM_HPP

#include <cmath>
#include <vector>

namespace resembla {

template<typename value_type = double>
class WeightedL2Norm
{
public:
    value_type operator()(const std::vector<value_type>& weights, const std::vector<value_type>& values) const
    {
        value_type total_weight{0}, norm{0};
        for(size_t i = 0; i < weights.size(); ++i){
            total_weight += weights[i];
            norm += weights[i] * values[i] * values[i];
        }
        return static_cast<value_type>(std::sqrt(norm) / total_weight);
    }
};

}
#endif
