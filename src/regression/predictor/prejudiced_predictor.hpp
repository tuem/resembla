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

#ifndef RESEMBLA_PREJUDICED_PREDICTOR_PREDICTOR_HPP
#define RESEMBLA_PREJUDICED_PREDICTOR_PREDICTOR_HPP

#include <string>

#include "../feature.hpp"

namespace resembla {

class PrejudicedPredictor
{
public:
    using input_type = FeatureMap;
    using output_type = Feature::real_type;

    static const std::string DEFAULT_KEY;

    PrejudicedPredictor(const std::string& key = DEFAULT_KEY);

    output_type operator()(const input_type& x) const;

protected:
    const std::string key;
};

}
#endif
