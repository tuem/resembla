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

#ifndef RESEMBLA_WORD_WEIGHT_HPP
#define RESEMBLA_WORD_WEIGHT_HPP

#include "../word.hpp"
#include "../string_util.hpp"

namespace resembla {

struct WordWeight
{
    const double base_weight;
    const double delete_insert_ratio;
    const double noun_coefficient;
    const double verb_coefficient;
    const double adj_coefficient;

    WordWeight(double base_weight = 1L, double delete_insert_ratio = 1L,
            double noun_coefficient = 1L, double verb_coefficient = 1L, double adj_coefficient = 1L);

    double operator()(const Word<string_type>& word, bool is_original = false,
            size_t total = -1, size_t position = -1) const;
};

}
#endif
