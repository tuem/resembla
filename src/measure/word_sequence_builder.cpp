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

#include "word_sequence_builder.hpp"
#include "../string_util.hpp"

namespace resembla {

const int WordSequenceBuilder::FEATURE_SIZE = 9;

WordSequenceBuilder::WordSequenceBuilder(const std::string mecab_options): tagger(MeCab::createTagger(mecab_options.c_str())) {}

WordSequenceBuilder::output_type WordSequenceBuilder::operator()(const string_type& text, bool) const
{
    std::string text_string = cast_string<std::string>(text);
    output_type s;
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
        while(feature.size() < FEATURE_SIZE){
            feature.push_back(string_type());
        }

        s.push_back({surface, feature});
    }
    return s;
}

string_type WordSequenceBuilder::index(const string_type& text) const
{
    return text;
}

}
