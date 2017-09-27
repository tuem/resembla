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

#include "resembla_with_id.hpp"

#include <fstream>
#include <algorithm>

namespace resembla {

ResemblaWithId::ResemblaWithId(const std::shared_ptr<ResemblaInterface> resembla,
        std::string corpus_path, size_t id_col, size_t text_col):
    resembla(resembla)
{
    loadCorpus(corpus_path, id_col, text_col);
}

std::vector<ResemblaWithId::output_type> ResemblaWithId::find(const string_type& query,
        double threshold, size_t max_response) const
{
    std::vector<output_type> results;
    for(auto raw_result: resembla->find(query, threshold, max_response)){
        results.push_back({raw_result, ids.at(raw_result.text)});
    }
    return results;
}

std::vector<ResemblaWithId::output_type> ResemblaWithId::eval(const string_type& query,
        const std::vector<string_type>& targets, double threshold, size_t max_response) const
{
    std::vector<output_type> results;
    for(auto raw_result: resembla->eval(query, targets, threshold, max_response)){
        auto i = ids.find(raw_result.text);
        results.push_back({raw_result, i != ids.end() ? i->second : id_type{0}});
    }
    return results;
}

void ResemblaWithId::loadCorpus(const std::string& corpus_path, size_t id_col, size_t text_col)
{
    id_type max_id = 0;
    std::ifstream ifs(corpus_path);
    if(ifs.fail()){
        throw std::runtime_error("input file is not available: " + corpus_path);
    }

    while(ifs.good()){
        std::string line;
        std::getline(ifs, line);
        if(ifs.eof() || line.length() == 0){
            break;
        }

        auto columns = split(line, column_delimiter<>());
        if(text_col - 1 < columns.size()){
            auto text = cast_string<string_type>(columns[text_col - 1]);
            auto i = ids.find(text);
            if(i == std::end(ids)){
                id_type id;
                if(id_col == 0 || id_col > columns.size()){
                    id = ++max_id;
                }
                else{
                    id = std::stoi(columns[id_col - 1]);
                    max_id = std::max(id, max_id);
                }
                ids[text] = id;
            }
        }
    }
}

}
