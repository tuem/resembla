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

#ifndef __ROMAJI_MATCH_WEIGHT_HPP__
#define __ROMAJI_MATCH_WEIGHT_HPP__

#include <unordered_set>

namespace resembla {

struct RomajiMatchWeight
{
    using value_type = wchar_t;

    double base_weight;
    double delete_insert_ratio;

    double uppercase_coefficient;
    double lowercase_coefficient;

    double vowel_coefficient;
    double consonant_coefficient;

    static const std::unordered_set<value_type> VOWELS;
    static const std::unordered_set<value_type> CONSONANTS;

    RomajiMatchWeight(double base_weight = 1L, double delete_insert_ratio = 1L, 
            double uppercase_coefficient = 1L, double lowercase_coefficient = 1L, 
            double vowel_coefficient = 1L, double consonant_coefficient = 1L);

    static bool isLower(value_type c);
    static bool isUpper(value_type c);
    static bool isVowel(value_type c);
    static bool isConsonant(value_type c);

    double operator()(const value_type c, bool is_original = false, size_t total = -1, size_t position = -1) const;
};

}
#endif
