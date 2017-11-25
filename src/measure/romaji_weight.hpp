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

#ifndef RESEMBLA_ROMAJI_WEIGHT_HPP
#define RESEMBLA_ROMAJI_WEIGHT_HPP

#include <unordered_set>

#include "../string_util.hpp"

namespace resembla {

template<typename string_type>
struct RomajiWeight
{
    using value_type = typename string_type::value_type;

    RomajiWeight(const string_type& vowels, const string_type& consonants,
            double base_weight = 1.0, double delete_insert_ratio = 1.0,
            double uppercase_coefficient = 1.0, double lowercase_coefficient = 1.0,
            double vowel_coefficient = 1.0, double consonant_coefficient = 1.0):
        base_weight(base_weight), delete_insert_ratio(delete_insert_ratio),
        uppercase_coefficient(uppercase_coefficient), lowercase_coefficient(lowercase_coefficient),
    vowel_coefficient(vowel_coefficient), consonant_coefficient(consonant_coefficient)
    {
        for(auto c: vowels){
            this->vowels.insert(c);
        }
        for(auto c: consonants){
            this->consonants.insert(c);
        }
        // TODO: define by constexpr
        auto u = cast_string<string_type>(std::string("AZ"));
        upper_begin = u[0];
        upper_end = u[1];
        auto l = cast_string<string_type>(std::string("az"));
        lower_begin = u[0];
        lower_end = u[1];
    }

    double operator()(const value_type c, bool is_original = false, size_t total = -1, size_t position = -1) const
    {
        (void)total;
        (void)position;

        double weight = base_weight;
        if(is_original){
            weight *= delete_insert_ratio;
        }

        if(isUpper(c)){
            weight *= uppercase_coefficient;
        }
        else if(isLower(c)){
            weight *= lowercase_coefficient;
        }

        if(isVowel(c)){
            weight *= vowel_coefficient;
        }
        else if(isConsonant(c)){
            weight *= consonant_coefficient;
        }

        return weight;
    }

protected:
    const double base_weight;
    const double delete_insert_ratio;

    const double uppercase_coefficient;
    const double lowercase_coefficient;

    const double vowel_coefficient;
    const double consonant_coefficient;

    value_type upper_begin;
    value_type upper_end;
    value_type lower_begin;
    value_type lower_end;

    std::unordered_set<value_type> vowels;
    std::unordered_set<value_type> consonants;

    bool isUpper(value_type c) const
    {
        return upper_begin <= c && c <= upper_end;
    }

    bool isLower(value_type c) const
    {
        return lower_begin <= c && c <= lower_end;
    }

    bool isVowel(value_type c) const
    {
        return vowels.count(c) > 0;
    }

    bool isConsonant(value_type c) const
    {
        return consonants.count(c) > 0;
    }
};

}
#endif
