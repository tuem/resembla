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

#ifndef RESEMBLA_FEATURE_HPP
#define RESEMBLA_FEATURE_HPP

#include <string>
#include <unordered_map>

namespace resembla {

struct Feature
{
    using key_type = std::string;
    using text_type = std::string;
    using real_type = double;

    const key_type key;
    const text_type value;

    static text_type toText(const real_type& a);
    static real_type toReal(const text_type& a);
};

using StringFeatureMap = std::unordered_map<Feature::key_type, Feature::text_type>;
using FeatureMap = std::unordered_map<Feature::key_type, Feature::real_type>;

}
#endif
