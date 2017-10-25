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

#include "romaji_mismatch_cost.hpp"

#include <algorithm>

#include "../csv_reader.hpp"

namespace resembla {

RomajiMismatchCost::RomajiMismatchCost(const std::string& letter_similarity_file_path,
        double case_mismatch_cost):
    case_mismatch_cost(case_mismatch_cost)
{
    for(const auto& columns: CsvReader<>(letter_similarity_file_path, 2)){
        auto letters = cast_string<string_type>(columns[0]);
        auto cost = std::stod(columns[1]);

        std::sort(std::begin(letters), std::end(letters));
        for(size_t i = 0; i < letters.size(); ++i){
            string_type p(1, letters[i]);
            for(size_t j = i + 1; j < letters.size(); ++j){
                letter_similarities[p + letters[j]] = cost;
            }
        }
    }
}

double RomajiMismatchCost::operator()(const value_type a, const value_type b) const
{
    if(a == b){
        return 0L;
    }

    double result = 1L;

    auto al = toLower(a), bl = toLower(b);
    if(al == bl){
        result = case_mismatch_cost;
    }
    else{
        if(bl < al){
            auto tmp = al;
            al = bl;
            bl = tmp;
        }
        auto p = letter_similarities.find(string_type({al, bl}));
        if(p != std::end(letter_similarities)){
            if((a == al && b == bl) || (a != al && b != bl)){
                result = p->second;
            }
            else{
                // special case that cases are different but letters are similar
                result = std::min(result, case_mismatch_cost + p->second);
            }
        }
    }

    return result;
}

RomajiMismatchCost::value_type RomajiMismatchCost::toLower(value_type a) const
{
    if(L'A' <= a && a <= L'Z'){
        return a + (L'a' - L'A');
    }
    else{
        return a;
    }
}

}
