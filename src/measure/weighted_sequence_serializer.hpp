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

#ifndef RESEMBLA_WEIGHTED_SEQUENCE_SERIALIZER_HPP
#define RESEMBLA_WEIGHTED_SEQUENCE_SERIALIZER_HPP

#include <json.hpp>

#include "weighted_sequence_builder.hpp"

#include "word_preprocessor.hpp"
#include "word_weight.hpp"
#include "pronunciation_preprocessor.hpp"
#include "letter_weight.hpp"
#include "romaji_preprocessor.hpp"
#include "romaji_weight.hpp"

namespace resembla {

/* TODO: implement as template functions if possible
template<typename SequenceTokenizer, typename WeightFunction>
void to_json(nlohmann::json& j, const typename WeightedSequenceBuilder<SequenceTokenizer, WeightFunction>::token_type& o) {
    j = nlohmann::json{{"token", cast_string<std::string>(string_type(1, o.token))}, {"weight", o.weight}};
}

template<typename SequenceTokenizer, typename WeightFunction>
void from_json(const nlohmann::json& j, typename WeightedSequenceBuilder<SequenceTokenizer, WeightFunction>::token_type& o) {
    o.token = cast_string<SequenceTokenizer::output_type>(j.at("token").get<std::string>())[0];
    o.weight = j.at("weight").get<double>();
}
*/

void to_json(nlohmann::json& j, const typename WeightedSequenceBuilder<WordPreprocessor<string_type>, WordWeight>::token_type& o);
void from_json(const nlohmann::json& j, typename WeightedSequenceBuilder<WordPreprocessor<string_type>, WordWeight>::token_type& o);

void to_json(nlohmann::json& j, const typename WeightedSequenceBuilder<PronunciationPreprocessor, LetterWeight<string_type>>::token_type& o);
void from_json(const nlohmann::json& j, typename WeightedSequenceBuilder<PronunciationPreprocessor, LetterWeight<string_type>>::token_type& o);

void to_json(nlohmann::json& j, const typename WeightedSequenceBuilder<RomajiPreprocessor, RomajiWeight>::token_type& o);
void from_json(const nlohmann::json& j, typename WeightedSequenceBuilder<RomajiPreprocessor, RomajiWeight>::token_type& o);

}
#endif
