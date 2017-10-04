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

#ifndef RESEMBLA_INTERVAL_FEATURE_AGGREGATOR_HPP
#define RESEMBLA_INTERVAL_FEATURE_AGGREGATOR_HPP

#include "feature_aggregator.hpp"

namespace resembla {

struct IntervalFeatureAggregator: public FeatureAggregator::Function
{
    Feature::real_type operator()(const Feature::text_type& a, const Feature::text_type& b) const;
};

}
#endif
