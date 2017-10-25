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

#include "romaji_weight.hpp"

namespace resembla {

const std::unordered_set<RomajiWeight::value_type> RomajiWeight::VOWELS = {
    L'A',
    L'E',
    L'I',
    L'O',
    L'U',
    L'a',
    L'e',
    L'i',
    L'o',
    L'u',
    L'-'
};

const std::unordered_set<RomajiWeight::value_type> RomajiWeight::CONSONANTS = {
    L'B',
    L'C',
    L'D',
    L'F',
    L'G',
    L'H',
    L'J',
    L'K',
    L'L',
    L'M',
    L'N',
    L'P',
    L'Q',
    L'R',
    L'S',
    L'T',
    L'V',
    L'W',
    L'X',
    L'Y',
    L'Z',
    L'b',
    L'c',
    L'd',
    L'f',
    L'g',
    L'h',
    L'j',
    L'k',
    L'l',
    L'm',
    L'n',
    L'p',
    L'q',
    L'r',
    L's',
    L't',
    L'v',
    L'w',
    L'x',
    L'y',
    L'z'
};

RomajiWeight::RomajiWeight(double base_weight, double delete_insert_ratio, 
        double uppercase_coefficient, double lowercase_coefficient, 
        double vowel_coefficient, double consonant_coefficient):
    base_weight(base_weight), delete_insert_ratio(delete_insert_ratio),
    uppercase_coefficient(uppercase_coefficient), lowercase_coefficient(lowercase_coefficient),
    vowel_coefficient(vowel_coefficient), consonant_coefficient(consonant_coefficient)
{}

double RomajiWeight::operator()(const value_type c, bool is_original, size_t, size_t) const
{
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

bool RomajiWeight::isUpper(const value_type c)
{
    return L'A' <= c && c <= L'Z';
}

bool RomajiWeight::isLower(const value_type c)
{
    return L'a' <= c && c <= L'z';
}

bool RomajiWeight::isVowel(const value_type c)
{
    return VOWELS.count(c) > 0;
}

bool RomajiWeight::isConsonant(const value_type c)
{
    return CONSONANTS.count(c) > 0;
}

}
