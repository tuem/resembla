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

#ifndef RESEMBLA_WORD_PREPROCESSOR_HPP
#define RESEMBLA_WORD_PREPROCESSOR_HPP

#include <memory>
#include <string>
#include <mutex>

#include <mecab.h>

#include "../word.hpp"
#include "../string_util.hpp"
#include "../mecab_util.hpp"

namespace resembla {

template<typename string_type>
class WordPreprocessor final
{
public:
    using token_type = Word<string_type>;
    using output_type = std::vector<token_type>;

    WordPreprocessor(const std::string& mecab_options = "", size_t min_feature_size = 9):
            tagger(MeCab::createTagger(validate_mecab_options(mecab_options).c_str())),
            min_feature_size(min_feature_size){}
    WordPreprocessor(const WordPreprocessor& obj) = default;

    // parses to a sequence of words
    output_type operator()(const string_type& text, bool is_original = false) const
    {
        (void)is_original;

        std::string text_string = cast_string<std::string>(text);
        output_type s;
        {
            std::lock_guard<std::mutex> lock(mutex_tagger);
            for(const MeCab::Node* node = tagger->parseToNode(text_string.c_str()); node; node = node->next){
                // skip BOS/EOS nodes
                if(node->stat == MECAB_BOS_NODE || node->stat == MECAB_EOS_NODE){
                    continue;
                }

                // extract surface and features
                string_type surface = cast_string<string_type>(std::string(node->surface, node->surface + node->length));
                std::vector<string_type> feature;
                const char *start = node->feature;
                for(const char* end = start; *end != '\0'; ++end){
                    if(*end == ','){
                        if(start < end){
                            feature.push_back(cast_string<string_type>(std::string(start, end)));
                        }
                        start = end + 1;
                    }
                }
                if(*start != '\0'){
                    feature.push_back(cast_string<string_type>(std::string(start)));
                }
                while(feature.size() < min_feature_size){
                    feature.push_back(string_type());
                }

                s.push_back({surface, feature});
            }
        }
        return s;
    }

protected:
    std::shared_ptr<MeCab::Tagger> tagger;
    mutable std::mutex mutex_tagger;

    const size_t min_feature_size;
};

}
#endif
