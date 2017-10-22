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

#include "interval_feature_aggregator.hpp"

#include "../../string_util.hpp"

namespace resembla {

Feature::real_type IntervalFeatureAggregator::operator()(const Feature::text_type& a, const Feature::text_type& b) const
{
    if(a.empty() || b.empty()){
        return 0.0;
    }

    auto u = split(a, value_delimiter<Feature::text_type::value_type>());
    auto v = split(b, value_delimiter<Feature::text_type::value_type>());
    if(u.size() == 1 && v.size() == 1){
        return 0.0;
    }
    else if(u.size() == 1 && v.size() > 1){
        auto t = Feature::toReal(u[0]);
        auto s = Feature::toReal(v[0]);
        auto e = Feature::toReal(v[1]);
        if(s <= e){
            return (s <= t && t <= e) ? 1.0 : -1.0;
        }
        else{
            return (s <= t || t <= e) ? 1.0 : -1.0;
        }
    }
    else if(v.size() == 1 && u.size() > 1){
        auto t = Feature::toReal(v[0]);
        auto s = Feature::toReal(u[0]);
        auto e = Feature::toReal(u[1]);
        if(s <= e){
            return (s <= t && t <= e) ? 1.0 : -1.0;
        }
        else{
            return (s <= t || t <= e) ? 1.0 : -1.0;
        }
    }
    else{
        auto s0 = Feature::toReal(u[0]);
        auto e0 = Feature::toReal(u[1]);
        auto s1 = Feature::toReal(v[0]);
        auto e1 = Feature::toReal(v[1]);
        if(s0 <= e0){
            if(s1 <= e1){
                return (s0 <= e1 && s1 <= e0) ? 1.0 : -1.0;
            }
            else{
                return (s0 <= e1 || s1 <= e0) ? 1.0 : -1.0;
            }
        }
        else{
            if(s1 <= e1){
                return (s1 <= e0 || s0 <= e1) ? 1.0 : -1.0;
            }
            else{
                return 1.0;
            }
        }
    }
}

}
