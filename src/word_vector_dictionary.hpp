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

#include "string_util.hpp"

namespace resembla {

template<typename string_type, typename value_type = double, typename id_type = unsigned long>
class WordVectorDictionary
{
public:
    WordVectorDictionary(const std::string& dictionary_path)
    {
        std::ifstream ifs(dictionary_path);
        if(ifs.fail()){
            throw std::runtime_error("input file is not available: " + dictionary_path);
        }

        id_type current_id = 0;
        while(true){
            std::string line;
            std::getline(ifs, line);
            if(ifs.eof()){
                break;
            }

            raw_values = split(line, ' ');
            string_type surface = cast_string<string_type>(raw_values[0]);
            std::vector<value_type> values;
            for(size_t i = 1; i < raw_values.size(); ++i){
                values.push_back(static_cast<value_type>(std::stod(raw_values[i])));
            }

            dictionary[current_id] = {current_id, surface, values};
            ++current_id;
        }
    }

    id_type id(const string_type& surface)
    {
        auto i = surface_ids.find(surface);
        return i != std::end(surface_ids) ? *i : -1;
    }

    const WordVector<string_type, value_type, id_type>& vector(id_type id)
    {
        return dictionary[id];
    }

private:
    size_t dimension;

    std::unordered_map<string_type, id_type> surface_ids;
    std::unordered_map<id_type, WordVector<string_type, value_type, id_type>> dictionary;
};

}
#endif
