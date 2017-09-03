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

#ifndef RESEMBLA_RESEMBLA_WITH_ID_HPP
#define RESEMBLA_RESEMBLA_WITH_ID_HPP

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <fstream>

#include "resembla_interface.hpp"
#include "regression/feature.hpp"

namespace resembla {

class ResemblaWithId
{
public:
    using id_type = int;

    struct output_type: public ResemblaInterface::output_type
    {
        id_type id;

        output_type(const ResemblaInterface::output_type& output, id_type id):
            ResemblaInterface::output_type(output), id(id)
        {}
    };

    ResemblaWithId(const std::shared_ptr<ResemblaInterface> resembla,
            std::string corpus_path, size_t id_col = 1, size_t text_col = 2);

    std::vector<output_type> find(const string_type& query, double threshold = 0.0,
            size_t max_response = 0) const;
    std::vector<output_type> eval(const string_type& query, const std::vector<string_type>& targets,
            double threshold = 0.0, size_t max_response = 0) const;

protected:
    std::shared_ptr<ResemblaInterface> resembla;
    std::unordered_map<string_type, id_type> ids;

    void loadCorpus(const std::string& corpus_path, size_t id_col, size_t text_col);
};

}
#endif
