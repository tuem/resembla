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

#ifndef RESEMBLA_PRONUNCIATION_PREPROCESSOR_HPP
#define RESEMBLA_PRONUNCIATION_PREPROCESSOR_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>

#include <mecab.h>

#include "../string_util.hpp"

namespace resembla {

struct PronunciationPreprocessor
{
    using string_type = resembla::string_type;
    using token_type = string_type::value_type;
    using output_type = string_type;

    PronunciationPreprocessor(const std::string& mecab_options = "",
            size_t mecab_feature_pos = 7, const std::string& mecab_pronunciation_of_marks = "");
    PronunciationPreprocessor(const PronunciationPreprocessor& obj) = default;
    virtual ~PronunciationPreprocessor() = default;

    output_type operator()(const string_type& text, bool is_original = false) const;

protected:
    static const std::unordered_map<token_type, string_type> KANA_MAP;

    std::shared_ptr<MeCab::Tagger> tagger;
    mutable std::mutex mutex_tagger;

    const size_t mecab_feature_pos;
    string_type mecab_pronunciation_of_marks;

    bool isKanaWord(const string_type& w) const;
    string_type estimatePronunciation(const string_type& w) const;
};

}
#endif
