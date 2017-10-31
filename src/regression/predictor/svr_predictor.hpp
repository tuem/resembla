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

#ifndef RESEMBLA_SVR_PREDICTOR_HPP
#define RESEMBLA_SVR_PREDICTOR_HPP

#include <string>
#include <vector>
#include <mutex>

#include <libsvm/svm.h>

#include "../feature.hpp"

namespace resembla {

// LIBSVM wrapper for Resembla
class SVRPredictor
{
public:
    using input_type = FeatureMap;
    using output_type = Feature::real_type;

    SVRPredictor(const std::vector<Feature::key_type>& feature_definitions,
            const std::string& model_file_path);
    SVRPredictor(const SVRPredictor& obj);
    virtual ~SVRPredictor();

    output_type operator()(const input_type& x) const;

protected:
    const std::vector<Feature::key_type> feature_definitions;

    svm_model *model;
    mutable std::mutex mutex_model;

    std::vector<svm_node> toNodes(const input_type& x) const;
};

}
#endif
