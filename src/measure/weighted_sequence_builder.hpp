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

#ifndef RESEMBLA_WEIGHTED_SEQUENCE_BUILDER_HPP
#define RESEMBLA_WEIGHTED_SEQUENCE_BUILDER_HPP

#include <vector>
#include <memory>

#include "../string_util.hpp"

namespace resembla {

template<typename SequenceTokenizer, typename WeightFunction>
class WeightedSequenceBuilder
{
public:
    struct token_type
    {
        typename SequenceTokenizer::token_type token;
        double weight;
    };
    using output_type = std::vector<token_type>;

    WeightedSequenceBuilder(const std::shared_ptr<SequenceTokenizer> tokenize,
            const std::shared_ptr<WeightFunction> weight_func):
        tokenize(tokenize), weight_func(weight_func) {}

    output_type operator()(const string_type& text, bool is_original = false) const
    {
        auto s = (*tokenize)(is_original ?
                split(text, column_delimiter<string_type::value_type>())[0] : text, is_original);
        output_type ws;
        for(size_t i = 0; i < s.size(); ++i){
            ws.push_back({s[i], (*weight_func)(s[i], is_original, s.size(), i)});
        }
        return ws;
    }

protected:
    std::shared_ptr<SequenceTokenizer> tokenize;
    std::shared_ptr<WeightFunction> weight_func;
};

}
#endif
