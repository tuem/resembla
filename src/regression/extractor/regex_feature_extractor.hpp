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

#ifndef RESEMBLA_REGEX_FEATURE_EXTRACTOR_HPP
#define RESEMBLA_REGEX_FEATURE_EXTRACTOR_HPP

#include <regex>
#include <string>
#include <vector>
#include <initializer_list>

#include "feature_extractor.hpp"
#include "../../string_util.hpp"

namespace resembla {

struct RegexFeatureExtractor: public FeatureExtractor::Function
{
    template<class ScorePatternPairs>
    RegexFeatureExtractor(const ScorePatternPairs& patterns)
    {
        construct(patterns);
    }

    RegexFeatureExtractor(const std::string& file_path);
    RegexFeatureExtractor(const std::initializer_list<std::pair<Feature::real_type, std::string>>& patterns);

    Feature::text_type operator()(const string_type& text) const;

protected:
    using regex = std::basic_regex<string_type::value_type>;

    std::vector<std::pair<Feature::real_type, regex>> re_all;

    template<class ScorePatternPairs>
    void construct(const ScorePatternPairs& patterns)
    {
        for(const auto& i: patterns){
            re_all.push_back(std::make_pair(i.first, regex(cast_string<string_type>(i.second))));
        }
    }

    std::vector<std::pair<double, std::string>> load(const std::string& file_path);

    Feature::real_type match(const string_type& text) const;
};

}
#endif
