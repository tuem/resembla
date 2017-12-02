/* Resembla https://github.com/tuem/resembla

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

#ifndef RESEMBLA_WORD_VECTOR_DICTIONARY_HPP
#define RESEMBLA_WORD_VECTOR_DICTIONARY_HPP

#include <string>
#include <unordered_map>
#include <fstream>
#include <cmath>

#include "word_vector.hpp"
#include "string_util.hpp"

#ifdef DEBUG
#include <iostream>
#endif

namespace resembla {

template<typename string_type, typename value_type = double, typename id_type = long>
class WordVectorDictionary
{
public:
    WordVectorDictionary(const std::string& dictionary_path)
    {
#ifdef DEBUG
        std::cerr << "load word vector dictionary: " << dictionary_path << std::endl;
#endif
        std::ifstream ifs(dictionary_path);
        if(ifs.fail()){
            throw std::runtime_error("input file is not available: " + dictionary_path);
        }

        for(id_type i = 0; ; ++i){
            std::string line;
            std::getline(ifs, line);
            if(ifs.eof()){
                break;
            }

            auto raw_values = split(line, ' ');
            while(raw_values.back().empty()){
                raw_values.pop_back();
            }
            string_type word = cast_string<string_type>(raw_values[0]);
            std::vector<value_type> values;
            double square_sum = 0.0;
            for(size_t j = 1; j < raw_values.size(); ++j){
                auto v = std::stod(raw_values[j]);
                square_sum += v * v;
                values.push_back(static_cast<value_type>(v));
            }

            word_ids[word] = i;
            dictionary[i] = {i, word, values, static_cast<value_type>(std::sqrt(square_sum))};
        }
#ifdef DEBUG
        std::cerr << "done" << std::endl;
#endif
    }

    id_type id(const string_type& word) const
    {
        auto i = word_ids.find(word);
        return i != std::end(word_ids) ? i->second : -1;
    }

    const WordVector<string_type, value_type, id_type>& vector(id_type id) const
    {
        return dictionary.at(id);
    }

private:
    std::unordered_map<string_type, id_type> word_ids;
    std::unordered_map<id_type, WordVector<string_type, value_type, id_type>> dictionary;
};

}
#endif
