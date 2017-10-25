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

#ifndef RESEMBLA_ROMAJI_MISMATCH_COST_HPP
#define RESEMBLA_ROMAJI_MISMATCH_COST_HPP

#include <string>
#include <unordered_set>
#include <unordered_map>

#include "../string_util.hpp"

namespace resembla {

struct RomajiMismatchCost
{
    using value_type = string_type::value_type;

    RomajiMismatchCost(const std::string& letter_similarity_file_path, double case_mismatch_cost = 1L);

    double operator()(const value_type reference, const value_type target) const;

protected:
    std::unordered_map<string_type, double> letter_similarities;
    const double case_mismatch_cost;

    value_type toLower(value_type a) const;
};

}
#endif
