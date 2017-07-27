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

#include "svr_predictor.hpp"

#include <algorithm>
#include <iostream>

namespace resembla {

const std::string SVRPredictor::DEFAULT_NAME = "svr";

SVRPredictor::SVRPredictor(const std::vector<Feature::key_type>& feature_definitions,
        const std::string model_file_path, const std::string name):
    name(name), feature_definitions(feature_definitions), model(svm_load_model(model_file_path.c_str()))
{}

SVRPredictor::~SVRPredictor()
{
    svm_free_and_destroy_model(&model);
}

SVRPredictor::output_type SVRPredictor::operator()(const input_type& x) const
{
    auto nodes = toNodes(x);
    double s = svm_predict(model, &nodes[0]);
#ifdef DEBUG
    std::cerr << "svr input:" << std::endl;
    for(const auto& n: nodes){
        std::cerr << "  index=" << n.index << ", value=" << n.value << std::endl;
    }
    std::cerr << "svr result=" << s << std::endl;
#endif
    return std::max(std::min(s, 1.0), 0.0);
}

std::vector<svm_node> SVRPredictor::toNodes(const input_type& x) const
{
    std::vector<svm_node> nodes(feature_definitions.size() + 1);
    size_t i = 0;
    for(; i < feature_definitions.size(); ++i){
        auto j = x.find(feature_definitions[i]);
        if(j != std::end(x)){
            nodes[i].index = i;
            nodes[i].value = j->second;
        }
    }
    nodes[i].index = -1; // end of features
    return nodes;
}

}
