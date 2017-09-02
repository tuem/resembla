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

#ifndef RESEMBLA_RESEMBLA_ENSEMBLE_HPP
#define RESEMBLA_RESEMBLA_ENSEMBLE_HPP

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

#include "resembla_interface.hpp"

namespace resembla {

class ResemblaEnsemble: public ResemblaInterface
{
public:
    ResemblaEnsemble(const std::string& measure_name, const size_t max_reranking_num = 0);

    void append(const std::shared_ptr<ResemblaInterface> resembla, const double weight = 1.0);

    std::vector<output_type> find(const string_type& input, double threshold = 0.0, size_t max_response = 0) const;
    std::vector<output_type> eval(const string_type& query, const std::vector<string_type>& targets,
            double threshold = 0.0, size_t max_response = 0) const;

protected:
    // name to be used in response
    const std::string measure_name;

    const size_t max_reranking_num;

    double total_weight;

    // pairs of Resembla and its weight
    std::vector<std::pair<std::shared_ptr<ResemblaInterface>, double>> resemblas;

    std::vector<output_type> eval(const std::unordered_map<string_type, double>& aggregated,
            double threshold, size_t max_response) const;
};

}
#endif
