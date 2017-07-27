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

#ifndef __RESEMBLA_REGRESSION_HPP__
#define __RESEMBLA_REGRESSION_HPP__

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <fstream>

#include "resembla_interface.hpp"
#include "regression/extractor/feature_extractor.hpp"
#include "reranker.hpp"

namespace resembla {

template<typename ScoreFunction>
class ResemblaRegression: public ResemblaInterface
{
public:
    ResemblaRegression(size_t max_candidate,
            std::shared_ptr<FeatureExtractor> feature_extractor, std::shared_ptr<ScoreFunction> score_func,
            std::string corpus_path = "", size_t feature_col = 2):
        max_candidate(max_candidate), preprocess(feature_extractor), score_func(score_func),
        reranker(), preprocess_corpus(!corpus_path.empty())
    {
        if(preprocess_corpus){
            loadCorpusFeatures(corpus_path, feature_col);
        }
    }

    void append(const std::string name, const std::shared_ptr<ResemblaInterface> resembla, bool is_primary = true)
    {
        resemblas[name] = resembla;
        if(is_primary && primary_resembla_name.empty()){
            primary_resembla_name = name;
        }
    }

    std::vector<response_type> find(const string_type& input, size_t max_response, double threshold)
    {
        std::vector<string_type> candidate_texts;
        std::unordered_map<string_type, StringFeatureMap> candidate_features;

        // primary resembla
        for(const auto& r: resemblas[primary_resembla_name]->find(input, max_candidate, threshold)){
            candidate_texts.push_back(r.text);
            candidate_features[r.text] = preprocess_corpus ? corpus_features[r.text] : (*preprocess)(r.text);
            candidate_features[r.text][primary_resembla_name] = Feature::toText(r.score);
        }

        // other resemblas
        for(const auto& p: resemblas){
            if(p.first == primary_resembla_name){
                continue;
            }
            for(auto r: p.second->eval(input, candidate_texts)){
                candidate_features[r.text][p.first] = Feature::toText(r.score);
            }
        }

        // prepare data for reranking
        std::vector<WorkData> candidates;
        for(const auto& c: candidate_features){
            candidates.push_back(std::make_pair(c.first, c.second));
        }
        WorkData input_data = std::make_pair(input, (*preprocess)(input));

        // rerank by its own metric
        std::vector<ResemblaInterface::response_type> results;
        for(const auto& r: reranker.rerank(input_data, std::begin(candidates), std::end(candidates), *score_func)){
            if(r.second < threshold || results.size() >= max_response){
                break;
            }
            results.push_back({r.first, score_func->name, r.second});
        }
        return results;
    }

    std::vector<response_type> eval(const string_type& query, const std::vector<string_type>& targets)
    {
        std::unordered_map<string_type, StringFeatureMap> candidate_features;
        for(const auto& t: targets){
            candidate_features[t] = preprocess_corpus ? corpus_features[t] : (*preprocess)(t);
        }

        for(const auto& p: resemblas){
            for(const auto& r: p.second->eval(query, targets)){
                candidate_features[r.text][p.first] = Feature::toText(r.score);
            }
        }

        // prepare data for reranking
        std::vector<WorkData> candidates;
        for(const auto& c: candidate_features){
            candidates.push_back(std::make_pair(c.first, c.second));
        }
        WorkData input_data = std::make_pair(query, (*preprocess)(query));

        // rerank by its own metric
        std::vector<ResemblaInterface::response_type> results;
        for(const auto& r: reranker.rerank(input_data, std::begin(candidates), std::end(candidates), *score_func)){
            results.push_back({r.first, score_func->name, r.second});
        }
        return results;
    }

protected:
    using WorkData = std::pair<string_type, typename FeatureExtractor::return_type>;

    std::unordered_map<std::string, std::shared_ptr<ResemblaInterface>> resemblas;
    std::string primary_resembla_name;
    const size_t max_candidate;

    const std::shared_ptr<FeatureExtractor> preprocess;
    const std::shared_ptr<ScoreFunction> score_func;
    const Reranker<string_type> reranker;

    const bool preprocess_corpus;
    std::unordered_map<string_type, typename FeatureExtractor::return_type> corpus_features;

    void loadCorpusFeatures(const std::string& corpus_path, size_t features_col)
    {
        std::ifstream ifs(corpus_path);
        if(ifs.fail()){
            throw std::runtime_error("input file is not available: " + corpus_path);
        }
        while(ifs.good()){
            std::string line;
            std::getline(ifs, line);
            if(ifs.eof() || line.length() == 0){
                break;
            }
            auto columns = split(line, '\t');
            if(features_col - 1 < columns.size()){
                corpus_features[cast_string<string_type>(columns[0])] = (*preprocess)(columns[0], columns[features_col - 1]);
            }
        }
    }
};

}
#endif
