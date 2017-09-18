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

#include <fstream>
#include <vector>

namespace resembla {

TextClassificationFeatureExtractor::TextClassificationFeatureExtractor(
        const std::string& mecab_options, const std::string& dict_path, const std::string& model_path):
    model(svm_load_model(model_path.c_str()))
{
    std::ifstream ifs(dict_path);
    if(ifs.fail()){
        throw std::runtime_error("input file is not available: " + dict_path);
    }

    while(ifs.good()){
        std::string line;
        std::getline(ifs, line);
        if(ifs.eof()){
            break;
        }

        auto i = line.find(column_delimiter<>());
        if(i != std::string::npos){
            word_ids[line.substr(0, i)] = std::stoi(line.substr(i + 1));
        }
    }
}

Feature::text_type TextClassificationFeatureExtractor::operator()(const string_type& text) const
{
    auto nodes = toNodes(text);
    double s;
    {
        std::lock_guard<std::mutex> lock(mutex_model);
        s = svm_predict(model, &nodes[0]);
    }
    return s;
}

std::vector<svm_node> TextClassificationFeatureExtractor::toNodes(const string_type& text) const
{
    std::unordered_map<int, int> word_counts;
    {
        std::lock_guard<std::mutex> lock(mutex_tagger);
        for(const MeCab::Node* node = tagger->parseToNode(text_string.c_str()); node; node = node->next){
            // skip BOS/EOS nodes
            if(node->stat == MECAB_BOS_NODE || node->stat == MECAB_EOS_NODE){
                continue;
            }

            auto i = word_ids.find(std::string(node->surface, node->surface + node->length));
            if(i != word_ids.end()){
                auto j = word_counts.find(*i);
                if(j == word_counts.end()){
                    word_counts[*i] = 1;
                }
                else{
                    ++*j;
                }
            }
        }
    }

    std::vector<svm_node> nodes(word_counts.size() + 1);
    size_t i = 0;
    for(const auto j: word_counts){
        nodes[j].index = j.first;
        nodes[j].value = static_cast<double>(j.second);
        ++i;
    }
    nodes[i].index = -1; // end of features
    return nodes;
}

}
#endif
