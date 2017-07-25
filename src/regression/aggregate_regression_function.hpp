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

#ifndef __AGGREGATE_REGRESSION_FUNCTION_HPP__
#define __AGGREGATE_REGRESSION_FUNCTION_HPP__

namespace resembla {

template<class AggregateFunction, class RegressionFunction>
class AggregateRegressionFunction
{
public:
    const std::string name;

    AggregateRegressionFunction(std::shared_ptr<AggregateFunction> aggregate_func, std::shared_ptr<RegressionFunction> regression_func):
        name(regression_func->name), aggregate_func(aggregate_func), regression_func(regression_func)
    {}

    double operator()(const StringFeatureMap& a, const StringFeatureMap& b) const
    {
        return (*regression_func)((*aggregate_func)(a, b));
    }

protected:
    std::shared_ptr<AggregateFunction> aggregate_func;
    std::shared_ptr<RegressionFunction> regression_func;
};

}
#endif
