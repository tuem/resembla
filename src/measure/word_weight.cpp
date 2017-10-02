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

#include "word_weight.hpp"

namespace resembla {

WordWeight::WordWeight(double base_weight, double delete_insert_ratio,
        double noun_coefficient, double verb_coefficient, double adj_coefficient):
    base_weight(base_weight), delete_insert_ratio(delete_insert_ratio),
    noun_coefficient(noun_coefficient), verb_coefficient(verb_coefficient),
    adj_coefficient(adj_coefficient) {}

double WordWeight::operator()(const Word<string_type>& word, bool is_original, size_t, size_t) const
{
    double weight = base_weight;

    // TODO: parameterize values
    if(word.feature[7].empty() || word.feature[7] == L"*"){
        weight *= word.surface.length();
    }
    else{
        weight *= word.feature[7].length();
    }

    if(is_original){
        weight *= delete_insert_ratio;
    }

    if(word.feature[0] == L"名詞" && word.feature[1] != L"接尾" && word.feature[1] != L"非自立" &&
            word.feature[1] != L"副詞可能" && word.feature[1] != L"代名詞"){
        weight *= noun_coefficient;
    }
    else if(word.feature[0] == L"動詞" && word.feature[1] != L"接尾" && word.feature[1] != L"非自立"){
        weight *= verb_coefficient;
    }
    else if(word.feature[0] == L"形容詞"){
        weight *= adj_coefficient;
    }

    return weight;
}

}
