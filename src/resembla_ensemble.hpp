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

#ifndef __RESEMBLA_ENSEMBLE_HPP__
#define __RESEMBLA_ENSEMBLE_HPP__

#include <vector>
#include <memory>

#include "resembla_interface.hpp"

namespace resembla {

class ResemblaEnsemble: public ResemblaInterface
{
public:
    ResemblaEnsemble(const std::string& measure_name);

    void append(const std::shared_ptr<ResemblaInterface> resembla, const double weight = 1.0);

    std::vector<response_type> getSimilarTexts(const string_type& input, size_t max_response = 20, double threshold = 0.0);

protected:
    // name to be used in response
    const std::string measure_name;

    // pairs of Resembla and its weight
    std::vector<std::pair<std::shared_ptr<ResemblaInterface>, double>> resemblas;
};

}
#endif
