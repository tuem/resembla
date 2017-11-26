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

#include "word_vector_preprocessor.hpp"

namespace resembla {

void to_json(nlohmann::json& j,
        const typename WordVectorPreprocessor<resembla::string_type, double, unsigned long>::token_type& o)
{
    j = nlohmann::json{{"i", o.id}, {"s", cast_string<std::string>(o.surface)}, {"v", o.values}, {"n", o.norm}};
}

void from_json(const nlohmann::json& j,
        typename WordVectorPreprocessor<resembla::string_type, double, unsigned long>::token_type& o)
{
    o.id = j.at("i").get<int>();
    o.surface = cast_string<string_type>(j.at("s").get<std::string>());
    o.values = j.at("v").get<std::vector<double>>();
    o.norm = j.at("n").get<double>();
}

}
