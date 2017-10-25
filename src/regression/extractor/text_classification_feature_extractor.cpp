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

#include "text_classification_feature_extractor.hpp"

#include "../../string_util.hpp"
#include "../../mecab_util.hpp"
#include "../../csv_reader.hpp"

namespace resembla {

TextClassificationFeatureExtractor::TextClassificationFeatureExtractor(
        const std::string& mecab_options, const std::string& dict_path, const std::string& model_path):
    tagger(MeCab::createTagger(validate_mecab_options(mecab_options).c_str())),
    model(svm_load_model(model_path.c_str()))
{
    for(const auto& columns: CsvReader<>(dict_path, 2)){
        dictionary[columns[1]] = std::stoi(columns[0]);
    }
}

Feature::text_type TextClassificationFeatureExtractor::operator()(const string_type& text) const
{
    auto nodes = toNodes(text);
    double s = 0.0;
    if(!nodes.empty()){
        {
            std::lock_guard<std::mutex> lock(mutex_model);
            s = svm_predict(model, &nodes[0]);
        }
    }
    return Feature::toText(s);
}

std::vector<svm_node> TextClassificationFeatureExtractor::toNodes(const string_type& text) const
{
    const auto text_string = cast_string<std::string>(text);
    BoW bow;
    {
        std::lock_guard<std::mutex> lock(mutex_tagger);
        for(const auto* node = tagger->parseToNode(text_string.c_str()); node; node = node->next){
            // skip BOS/EOS nodes
            if(node->stat == MECAB_BOS_NODE || node->stat == MECAB_EOS_NODE){
                continue;
            }

            auto i = dictionary.find(std::string(node->surface, node->surface + node->length));
            if(i == dictionary.end()){
                continue;
            }

            auto j = bow.find(i->second);
            if(j == bow.end()){
                bow[i->second] = 1;
            }
            else{
                ++j->second;
            }
        }
    }

    std::vector<svm_node> nodes(bow.size() + 1);
    size_t i = 0;
    for(const auto& j: bow){
        nodes[i].index = j.first;
        nodes[i].value = static_cast<double>(j.second);
        ++i;
    }
    nodes[i].index = -1; // end of features
    return nodes;
}

}
