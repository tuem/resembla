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

#ifndef RESEMBLA_WORD_VECTOR_PREPROCESSOR_HPP
#define RESEMBLA_WORD_VECTOR_PREPROCESSOR_HPP

#include <memory>
#include <string>
#include <mutex>

#include <mecab.h>
#include <json.hpp>

#include "../word_vector_dictionary.hpp"
#include "../string_util.hpp"
#include "../mecab_util.hpp"

namespace resembla {

template<typename string_type, typename value_type = double, typename id_type = long>
class WordVectorPreprocessor final
{
public:
    using token_type = WordVector<string_type, value_type, id_type>;
    using output_type = std::vector<token_type>;

    WordVectorPreprocessor(
            std::shared_ptr<WordVectorDictionary<string_type, value_type, id_type>> dictionary,
            const std::string& mecab_options = "", int word_pos = -1):
        dictionary(dictionary),
        tagger(MeCab::createTagger(validate_mecab_options(mecab_options).c_str())),
        word_pos(word_pos)
    {}

    WordVectorPreprocessor(const WordVectorPreprocessor& obj) = default;

    // parses to a sequence of words
    output_type operator()(const string_type& text, bool is_original = false) const
    {
        (void)is_original;

        auto text_string = cast_string<std::string>(
                is_original ? split(text, column_delimiter<typename string_type::value_type>())[0] : text);
        output_type sequence;
        {
            std::lock_guard<std::mutex> lock(mutex_tagger);
            for(const MeCab::Node* node = tagger->parseToNode(text_string.c_str()); node; node = node->next){
                // skip BOS/EOS nodes
                if(node->stat == MECAB_BOS_NODE || node->stat == MECAB_EOS_NODE){
                    continue;
                }

                std::string word;
                if(word_pos == -1){
                    word = std::string(node->surface, node->surface + node->length);
                }
                else{
                    const char *start = node->feature;
                    int i = 0;
                    for(const char* end = start; *end != '\0'; ++end){
                        if(*end == ','){
                            if(i++ == word_pos){
                                word = std::string(start, end);
                                break;
                            }
                            start = end + 1;
                        }
                    }
                    if(i == word_pos && *start != '\0'){
                        word = std::string(start);
                    }
                }
                auto id = dictionary->id(cast_string<string_type>(word));
                if(id == -1){
                    continue;
                }
                sequence.push_back(dictionary->vector(id));
            }
        }
        return sequence;
    }

protected:
    std::shared_ptr<WordVectorDictionary<string_type, value_type, id_type>> dictionary;
    std::shared_ptr<MeCab::Tagger> tagger;
    mutable std::mutex mutex_tagger;

    const int word_pos;
};

// TODO: implement as template functions
void to_json(nlohmann::json& j,
        const typename WordVectorPreprocessor<resembla::string_type, double, long>::token_type& o);
void from_json(const nlohmann::json& j,
        typename WordVectorPreprocessor<resembla::string_type, double, long>::token_type& o);

}
#endif
