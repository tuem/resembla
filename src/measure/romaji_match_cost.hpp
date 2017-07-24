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

#ifndef __ROMAJI_MATCH_COST_HPP__
#define __ROMAJI_MATCH_COST_HPP__

#include <string>
#include <unordered_set>

#include "../string_util.hpp"

namespace resembla {

struct RomajiMatchCost
{
    using value_type = string_type::value_type;

    static const std::unordered_set<string_type> SIMILAR_LETTER_PAIRS;

    const double case_mismatch_cost;
    const double similar_letter_cost;

    RomajiMatchCost(double case_mismatch_cost = 1L, double similar_letter_cost = 1L);

    value_type toLower(value_type a) const;

    double operator()(const value_type reference, const value_type target) const;
};

}
#endif
