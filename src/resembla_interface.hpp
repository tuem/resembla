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

#ifndef RESEMBLA_RESEMBLA_INTERFACE_HPP
#define RESEMBLA_RESEMBLA_INTERFACE_HPP

#include <vector>

#include "resembla_response.hpp"
#include "string_util.hpp"

namespace resembla {

class ResemblaInterface
{
public:
    using output_type = ResemblaResponse<string_type>;

    virtual ~ResemblaInterface() = default;
    virtual std::vector<output_type> find(const string_type& input,
            double threshold = 0.0, size_t max_response = 0) const = 0;
    virtual std::vector<output_type> eval(const string_type& input, const std::vector<string_type>& candidates,
            double threshold = 0.0, size_t max_response = 0) const = 0;
};

}
#endif
