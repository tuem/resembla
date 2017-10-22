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

#ifndef RESEMBLA_FEATURE_EXTRACTOR_HPP
#define RESEMBLA_FEATURE_EXTRACTOR_HPP

#include <unordered_map>
#include <memory>
#include <initializer_list>

#include "../../resembla_interface.hpp"
#include "../../string_util.hpp"
#include "../feature.hpp"

namespace resembla {

class FeatureExtractor
{
public:
    using string_type = resembla::string_type;
    using output_type = StringFeatureMap;

    struct Function
    {
        virtual ~Function() = default;
        virtual Feature::text_type operator()(const string_type& text) const = 0;
    };

    template<class F>
    struct StringToStringFunction: public Function
    {
        StringToStringFunction(){}
        StringToStringFunction(F f): f(f){}
        virtual ~StringToStringFunction() = default;

        Feature::text_type operator()(const string_type& text) const
        {
            return Feature::toText(f(text));
        }

    private:
        const F f;
    };

    FeatureExtractor(const std::string& base_similarity_key = "base_similarity");

    void append(Feature::key_type key, std::shared_ptr<Function> func);

    // load features of corpus texts
    output_type operator()(const std::string& raw_text, const std::string& raw_features) const;
    // extract features from corpus text using already loaded features
    output_type operator()(const ResemblaInterface::output_type& data, const output_type& given_features) const;
    // extract features from unknown text
    output_type operator()(const string_type& text) const;

    // extract features when indexing
    output_type operator()(const string_type& text, bool is_original) const;
    string_type index(const string_type& text) const;
protected:
    const std::string base_similarity_key;

    std::unordered_map<Feature::key_type, std::shared_ptr<Function>> functions;
};

}
#endif
