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

#ifndef __ROMAJI_SEQUENCE_BUILDER_HPP__
#define __ROMAJI_SEQUENCE_BUILDER_HPP__

#include <string>
#include <unordered_map>

#include "pronunciation_sequence_builder.hpp"

namespace resembla {

class RomajiSequenceBuilder: public PronunciationSequenceBuilder
{
public:
    RomajiSequenceBuilder(const std::string mecab_options = "", const size_t mecab_feature_pos = 7,
            const std::string mecab_pronunciation_of_marks = "", bool keep_case = false);

    output_type operator()(const string_type& text, bool is_original = false) const;

    string_type buildIndexingText(const string_type& text) const;

protected:
    static const std::unordered_map<string_type, string_type> ROMAJI_MAP;

    bool keep_case;
};

}
#endif
