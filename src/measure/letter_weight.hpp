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

#ifndef RESEMBLA_LETTER_WEIGHT_HPP
#define RESEMBLA_LETTER_WEIGHT_HPP

#include <string>
#include <fstream>
#include <exception>
#include <unordered_map>

#include "../string_util.hpp"

namespace resembla {

// simple weight function for sets of letters
template<typename string_type>
struct LetterWeight
{
    using value_type = typename string_type::value_type;

    LetterWeight(double base_weight, double delete_insert_ratio, const std::string& letter_weight_file_path):
        base_weight(base_weight), delete_insert_ratio(delete_insert_ratio)
    {
        if(letter_weight_file_path.empty()){
            return;
        }

        std::basic_ifstream<value_type> ifs(letter_weight_file_path);
        if(ifs.fail()){
            throw std::runtime_error("input file is not available: " + letter_weight_file_path);
        }

        while(ifs.good()){
            string_type line;
            std::getline(ifs, line);
            if(ifs.eof() || line.length() == 0){
                break;
            }

            auto columns = split(line, column_delimiter<value_type>());
            if(columns.size() < 2){
                throw std::runtime_error("invalid line in " + letter_weight_file_path + ": " + cast_string<std::string>(line));
            }

            auto letters= columns[0];
            auto weight = std::stod(columns[1]);
            for(size_t i = 0; i < letters.size(); ++i){
                letter_weights[letters[i]] = weight;
            }
        }
    }

    double operator()(const value_type c, bool is_original = false, size_t total = -1, size_t position = -1) const
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
    double base_weight;
    double delete_insert_ratio;

    std::unordered_map<value_type, double> letter_weights;
};

}
#endif
