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

#ifndef __RESEMBLA_WITH_ID_HPP__
#define __RESEMBLA_WITH_ID_HPP__

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <fstream>

#include "resembla_interface.hpp"
#include "regression/feature.hpp"

namespace resembla {

template<typename id_type>
class ResemblaWithId
{
public:
    struct output_type: public ResemblaInterface::output_type
    {
        id_type id;

        output_type(const ResemblaInterface::output_type& output, id_type id):
            ResemblaInterface::output_type(output), id(id)
        {}
    };

    ResemblaWithId(const std::shared_ptr<ResemblaInterface> resembla,
            std::string corpus_path, size_t id_col = 1, size_t text_col = 2):
        resembla(resembla)
    {
        loadCorpus(corpus_path, id_col, text_col);
    }

    std::vector<output_type> find(const string_type& query, double threshold = 0.0, size_t max_response = 0)
    {
        std::vector<output_type> results;
        for(auto raw_result: resembla->find(query, threshold, max_response)){
            results.push_back({raw_result, ids.at(raw_result.text)});
        }
        return results;
    }

    std::vector<output_type> eval(const string_type& query, const std::vector<string_type>& targets,
            double threshold = 0.0, size_t max_response = 0) const
    {
        std::vector<output_type> results;
        for(auto raw_result: resembla->find(query, targets, threshold, max_response)){
            results.push_back({raw_result, ids.at(raw_result.text)});
        }
        return results;
    }

protected:
    std::shared_ptr<ResemblaInterface> resembla;
    std::unordered_map<string_type, id_type> ids;

    id_type read_id(const std::string& id_str);

    void loadCorpus(const std::string& corpus_path, size_t id_col, size_t text_col)
    {
        auto c = std::max(id_col, text_col);
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
            auto columns = split(line, '\t');
            if(c - 1 < columns.size()){
                std::cerr << "text=" << columns[text_col - 1] << ", id=" << columns[id_col - 1] << std::endl;
                ids[cast_string<string_type>(columns[text_col - 1])] = read_id(columns[id_col - 1]);
            }
        }
    }
};

}
#endif
