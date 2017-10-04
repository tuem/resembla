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

#include "regex_feature_extractor.hpp"

#include "../../csv_reader.hpp"

#ifdef DEBUG
#include <iostream>
#endif

namespace resembla {

RegexFeatureExtractor::RegexFeatureExtractor(const std::initializer_list<std::pair<Feature::real_type, std::string>>& patterns)
{
    construct(patterns);
}

RegexFeatureExtractor::RegexFeatureExtractor(const std::string& file_path)
{
    construct(load(file_path));
}

Feature::real_type RegexFeatureExtractor::match(const string_type& text) const
{
    for(const auto& i: re_all){
        if(std::regex_match(text, i.second)){
#ifdef DEBUG
            std::cerr << "regex detector: evidence found, text=" << cast_string<std::string>(text) << ", score=" << i.first << std::endl;
#endif
            return i.first;
        }
    }
    return 0.0;
}

std::vector<std::pair<double, std::string>> RegexFeatureExtractor::load(const std::string& file_path)
{
    std::vector<std::pair<double, std::string>> patterns;
    for(const auto& columns: CsvReader<>(file_path, 2)){
        patterns.push_back(std::make_pair(std::stod(columns[0]), columns[1]));
    }
    return patterns;
}

Feature::text_type RegexFeatureExtractor::operator()(const string_type& text) const
{
    return Feature::toText(match(text));
}

}
