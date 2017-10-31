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

#include "svr_predictor.hpp"

#include <algorithm>

#ifdef DEBUG
#include <iostream>
#endif

namespace resembla {

SVRPredictor::SVRPredictor(const std::vector<Feature::key_type>& feature_definitions,
        const std::string& model_file_path):
    feature_definitions(feature_definitions), model(svm_load_model(model_file_path.c_str()))
{}

SVRPredictor::SVRPredictor(const SVRPredictor& obj):
    feature_definitions(obj.feature_definitions), model(obj.model), mutex_model()
{}

SVRPredictor::~SVRPredictor()
{
    svm_free_and_destroy_model(&model);
}

SVRPredictor::output_type SVRPredictor::operator()(const input_type& x) const
{
    auto nodes = toNodes(x);
    double s;
    {
        std::lock_guard<std::mutex> lock(mutex_model);
        s = svm_predict(model, &nodes[0]);
    }
#ifdef DEBUG
    std::cerr << "svm output=" << s << std::endl;
#endif
    return s;
}

std::vector<svm_node> SVRPredictor::toNodes(const input_type& x) const
{
    std::vector<svm_node> nodes(feature_definitions.size() + 1);
    size_t i = 0;
    for(size_t j = 0; j < feature_definitions.size(); ++j){
        auto k = x.find(feature_definitions[j]);
        if(k != std::end(x)){
            nodes[i].index = j;
            nodes[i].value = k->second;
            ++i;
        }
    }
    nodes[i].index = -1; // end of features
#ifdef DEBUG
    std::cerr << "svm inputs:" << std::endl;
    for(size_t j = 0; j < feature_definitions.size(); ++j){
        auto k = x.find(feature_definitions[j]);
        if(k != std::end(x)){
            std::cerr << "  feature: key=" << feature_definitions[j] << ", index=" << j << ", value=" << k->second << std::endl;
        }
        else{
            std::cerr << "  feature: key=" << feature_definitions[j] << ", index=" << j << ", value not found" << std::endl;
        }
    }
#endif
    return nodes;
}

}
