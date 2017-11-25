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

#ifndef RESEMBLA_LETTER_WEIGHT_HPP
#define RESEMBLA_LETTER_WEIGHT_HPP

#include <string>
#include <unordered_map>

#include "../string_util.hpp"
#include "../csv_reader.hpp"

namespace resembla {

// simple weight function for sets of letters
template<typename string_type>
struct LetterWeight
{
    using value_type = typename string_type::value_type;

    LetterWeight(double base_weight, double delete_insert_ratio,
            const std::string& letter_weight_file_path):
        base_weight(base_weight), delete_insert_ratio(delete_insert_ratio)
    {
        if(letter_weight_file_path.empty()){
            return;
        }

        for(const auto& columns: CsvReader<>(letter_weight_file_path, 2)){
            auto letters = cast_string<string_type>(columns[0]);
            auto weight = std::stod(columns[1]);

            for(auto c: letters){
                letter_weights[c] = weight;
            }
        }
    }

    double operator()(const value_type c, bool is_original = false,
            size_t total = -1, size_t position = -1) const
    {
        (void)total;
        (void)position;

        double w = base_weight;
        if(is_original){
            w *= delete_insert_ratio;
        }

        auto p = letter_weights.find(c);
        if(p != std::end(letter_weights)){
            w *= p->second;
        }

        return w;
    }

protected:
    const double base_weight;
    const double delete_insert_ratio;

    std::unordered_map<value_type, double> letter_weights;
};

}
#endif
