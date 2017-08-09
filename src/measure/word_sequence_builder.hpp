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

#ifndef __WORD_SEQUENCE_BUILDER_HPP__
#define __WORD_SEQUENCE_BUILDER_HPP__

#include <memory>
#include <string>
#include <mutex>

#include <mecab.h>

#include "../word.hpp"

namespace resembla {

class WordSequenceBuilder final
{
public:
    using token_type = Word;
    using output_type = std::vector<token_type>;

    WordSequenceBuilder(const std::string mecab_options = "");
    WordSequenceBuilder(const WordSequenceBuilder& obj);

    // parses to a sequence of words
    output_type operator()(const string_type& text, bool is_original = false) const;

    // returns text as-is
    string_type index(const string_type& text) const;

protected:
    static const int FEATURE_SIZE;
    std::shared_ptr<MeCab::Tagger> tagger;

    mutable std::mutex mutex_tagger;
};

}
#endif
