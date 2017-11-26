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
            const std::string& mecab_options = "", size_t min_feature_size = 9):
        dictionary(dictionary),
        tagger(MeCab::createTagger(validate_mecab_options(mecab_options).c_str())),
        min_feature_size(min_feature_size)
    {}

    WordVectorPreprocessor(const WordVectorPreprocessor& obj) = default;

    // parses to a sequence of words
    output_type operator()(const string_type& text, bool is_original = false) const
    {
        (void)is_original;

        std::string text_string = cast_string<std::string>(text);
        output_type sequence;
        {
            std::lock_guard<std::mutex> lock(mutex_tagger);
            for(const MeCab::Node* node = tagger->parseToNode(text_string.c_str()); node; node = node->next){
                // skip BOS/EOS nodes
                if(node->stat == MECAB_BOS_NODE || node->stat == MECAB_EOS_NODE){
                    continue;
                }

                // extract surface and features
                auto surface = cast_string<string_type>(std::string(node->surface, node->surface + node->length));
                auto id = dictionary->id(surface);
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

    const size_t min_feature_size;
};

// TODO: implement as template functions
void to_json(nlohmann::json& j,
        const typename WordVectorPreprocessor<resembla::string_type, double, long>::token_type& o);
void from_json(const nlohmann::json& j,
        typename WordVectorPreprocessor<resembla::string_type, double, long>::token_type& o);

}
#endif
