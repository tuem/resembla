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

#include <json.hpp>

#include "resembla_interface.hpp"
#include "reranker.hpp"
#include "regression/feature.hpp"
#include "regression/extractor/feature_extractor.hpp"

namespace resembla {

template<typename ScoreFunction>
class ResemblaRegression: public ResemblaInterface
{
public:
    ResemblaRegression(size_t max_candidate,
            std::shared_ptr<FeatureExtractor> feature_extractor, std::shared_ptr<ScoreFunction> score_func,
            std::string inverse_path = ""):
        max_candidate(max_candidate), preprocess(feature_extractor), score_func(score_func),
        reranker(), preprocess_corpus(!inverse_path.empty())
    {
        if(preprocess_corpus){
            loadCorpusFeatures(inverse_path);
        }
    }

    void append(const std::string name, const std::shared_ptr<ResemblaInterface> resembla, bool is_primary = true)
    {
        resemblas[name] = resembla;
        if(is_primary && primary_resembla_name.empty()){
            primary_resembla_name = name;
        }
    }

    std::vector<output_type> find(const string_type& query, double threshold = 0.0, size_t max_response = 0) const
    {
        std::vector<string_type> candidate_texts;
        std::unordered_map<string_type, StringFeatureMap> candidate_features;

        // primary resembla
        for(const auto& r: resemblas.at(primary_resembla_name)->find(query, threshold / 2.0, max_candidate * 2)){
            candidate_texts.push_back(r.text);
            candidate_features[r.text] = preprocess_corpus ? corpus_features.at(r.text) : (*preprocess)(r.text);
            candidate_features[r.text][primary_resembla_name] = Feature::toText(r.score);
        }

        // other resemblas
        for(const auto& p: resemblas){
            if(p.first == primary_resembla_name){
                continue;
            }
            for(auto r: p.second->eval(query, candidate_texts, 0.0, 0)){
                candidate_features[r.text][p.first] = Feature::toText(r.score);
                candidate_features[r.text][p.first] = Feature::toText(r.score);
            }
        }

        return eval(query, candidate_features, threshold, max_response);
    }

    std::vector<output_type> eval(const string_type& query, const std::vector<string_type>& targets, double threshold = 0.0, size_t max_response = 0) const
    {
        std::unordered_map<string_type, StringFeatureMap> candidate_features;
        for(const auto& t: targets){
            if(preprocess_corpus){
                auto i = corpus_features.find(t);
                if(i != std::end(corpus_features)){
                    candidate_features[t] = i->second;
                    continue;
                }
            }
            candidate_features[t] = (*preprocess)(t);
        }

        for(const auto& p: resemblas){
            for(const auto& r: p.second->eval(query, targets, 0.0, 0)){
                candidate_features[r.text][p.first] = Feature::toText(r.score);
            }
        }

        return eval(query, candidate_features, threshold, max_response);
    }

protected:
    using WorkData = std::pair<string_type, typename FeatureExtractor::output_type>;

    std::unordered_map<std::string, std::shared_ptr<ResemblaInterface>> resemblas;
    std::string primary_resembla_name;
    const size_t max_candidate;

    const std::shared_ptr<FeatureExtractor> preprocess;
    const std::shared_ptr<ScoreFunction> score_func;
    const Reranker<string_type> reranker;

    const bool preprocess_corpus;
    std::unordered_map<string_type, typename FeatureExtractor::output_type> corpus_features;

    void loadCorpusFeatures(const std::string& inverse_path)
    {
        std::ifstream ifs(inverse_path);
        if(ifs.fail()){
            throw std::runtime_error("input file is not available: " + inverse_path);
        }

        while(ifs.good()){
            std::string line;
            std::getline(ifs, line);
            if(ifs.eof() || line.length() == 0){
                break;
            }
            auto columns = split(line, '\t');
            if(columns.size() > 1){
                if(columns.size() > 2){
#ifdef DEBUG
                    std::cerr << "load from JSON: " << columns[2] << std::endl;
#endif
                    nlohmann::json j = nlohmann::json::parse(cast_string<std::string>(columns[2]));
                    typename FeatureExtractor::output_type preprocessed;
                    for(nlohmann::json::iterator i = std::begin(j); i != std::end(j); ++i){
                        preprocessed[i.key()] = i.value();
                    }
                    nlohmann::json j1 = preprocessed;
                    corpus_features[cast_string<string_type>(columns[1])] = preprocessed;
                }
                else{
#ifdef DEBUG
                    std::cerr << "preprocess: " << columns[2] << std::endl;
#endif
                    corpus_features[cast_string<string_type>(columns[1])] =
                        (*preprocess)(columns[1], "");
                }
            }
        }
    }

    std::vector<output_type> eval(const string_type& query,
            const std::unordered_map<string_type, StringFeatureMap>& candidate_features,
            double threshold, size_t max_response) const
    {
        // prepare data for reranking
        std::vector<WorkData> candidates;
        for(const auto& c: candidate_features){
            candidates.push_back(std::make_pair(c.first, c.second));
        }
        WorkData input_data = std::make_pair(query, (*preprocess)(query));

        // rerank by its own metric
        std::vector<ResemblaInterface::output_type> results;
        for(const auto& r: reranker.rerank(input_data, std::begin(candidates), std::end(candidates), *score_func, threshold, max_response)){
            results.push_back({r.first, score_func->name, std::max(std::min(r.second, 1.0), 0.0)});
        }
        return results;
    }
};

}
#endif
