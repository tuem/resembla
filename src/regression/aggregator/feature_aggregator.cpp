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

#include "feature_aggregator.hpp"

#ifdef DEBUG
#include <iostream>
#endif

namespace resembla {

void FeatureAggregator::append(Feature::key_type key, std::shared_ptr<Function> func)
{
    functions[key] = func;
}

FeatureAggregator::output_type FeatureAggregator::operator()(const input_type& a, const input_type& b) const
{
    output_type features;
    for(const auto& i: functions){
        auto j = a.find(i.first);
        auto k = b.find(i.first);
        if(j != std::end(a)){
            if(k != std::end(b)){
                // apply function if both a and b have values
                features[i.first] = (*i.second)(j->second, k->second);
            }
            else{
                features[i.first] = Feature::toReal(j->second);
            }
        }
        else if(k != std::end(b)){
            features[i.first] = Feature::toReal(k->second);
        }
    }
#ifdef DEBUG
    std::cerr << "given feature sets" << std::endl;
    for(const auto& i: functions){
        auto j = a.find(i.first);
        auto k = b.find(i.first);
        std::cerr << "  key=" << i.first << ", a=" << (j != std::end(a) ? j->second : "(none)") <<
            ", b=" << (k != std::end(b) ? k->second : "(none)") << std::endl;
    }
    std::cerr << "aggregated features" << std::endl;
    for(auto f: features){
        std::cerr << "  key=" << f.first << ", value=" << f.second << std::endl;
    }
#endif
    return features;
}

}
