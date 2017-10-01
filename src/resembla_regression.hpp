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

#ifndef RESEMBLA_RESEMBLA_REGRESSION_HPP
#define RESEMBLA_RESEMBLA_REGRESSION_HPP

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

#include <simstring/simstring.h>
#include <json.hpp>

#include "resembla_interface.hpp"
#include "csv_reader.hpp"
#include "eliminator.hpp"
#include "reranker.hpp"

#include "regression/feature.hpp"
#include "regression/extractor/feature_extractor.hpp"

namespace resembla {

template<typename Indexer, typename ScoreFunction>
class ResemblaRegression: public ResemblaInterface
{
public:
    ResemblaRegression(
            const std::string& db_path, const std::string& inverse_path,
            int simstring_measure, double simstring_threshold, size_t max_candidate,
            std::shared_ptr<Indexer> indexer, std::shared_ptr<FeatureExtractor> feature_extractor,
            std::shared_ptr<ScoreFunction> score_func):
        simstring_measure(simstring_measure), simstring_threshold(simstring_threshold), max_candidate(max_candidate),
        indexer(indexer), preprocess(feature_extractor), score_func(score_func), reranker()
    {
        db.open(db_path);
        load(inverse_path);
    }

    void append(const std::string& name, const std::shared_ptr<ResemblaInterface> resembla, bool is_primary = true)
    {
        resemblas[name] = resembla;
        if(is_primary && primary_resembla_name.empty()){
            primary_resembla_name = name;
        }
    }

    std::vector<output_type> find(const string_type& query, double threshold = 0.0, size_t max_response = 0) const
    {
        string_type search_query = indexer->index(query);

        // search from N-gram index
        std::vector<string_type> simstring_result;
        {
            std::lock_guard<std::mutex> lock(mutex_simstring);
            db.retrieve(search_query, simstring_measure, simstring_threshold, std::back_inserter(simstring_result));
        }
        if(simstring_result.empty()){
            return {};
        }
        else if(simstring_result.size() > max_candidate){
            Eliminator<string_type> eliminate(search_query);
            eliminate(simstring_result, max_candidate, true);
        }

        // load original texts
        std::vector<string_type> candidate_texts;
        for(const auto& i: simstring_result){
            if(i.empty()){
                continue;
            }
            const auto& j = inverse.at(i);
            std::copy(std::begin(j), std::end(j), std::back_inserter(candidate_texts));
        }

        // load pre-computed features
        std::unordered_map<string_type, StringFeatureMap> candidate_features;
        for(const auto& c: candidate_texts){
            candidate_features[c] = corpus_features.at(c);
        }

        // compute similarity using child Resembla
        for(const auto& p: resemblas){
            for(auto r: p.second->eval(query, candidate_texts, 0.0, 0)){
                candidate_features[r.text][p.first] = Feature::toText(r.score);
            }
        }

        return eval(query, candidate_features, threshold, max_response);
    }

    std::vector<output_type> eval(const string_type& query, const std::vector<string_type>& candidates,
            double threshold = 0.0, size_t max_response = 0) const
    {
        std::unordered_map<string_type, StringFeatureMap> candidate_features;
        for(const auto& c: candidates){
            auto i = corpus_features.find(c);
            if(i != std::end(corpus_features)){
                candidate_features[c] = i->second;
                continue;
            }
            candidate_features[c] = (*preprocess)(c);
        }

        for(const auto& p: resemblas){
            for(const auto& r: p.second->eval(query, candidates, 0.0, 0)){
                candidate_features[r.text][p.first] = Feature::toText(r.score);
            }
        }

        return eval(query, candidate_features, threshold, max_response);
    }

protected:
    using WorkData = std::pair<string_type, typename FeatureExtractor::output_type>;

    mutable simstring::reader db;
    std::unordered_map<string_type, std::vector<string_type>> inverse;

    const int simstring_measure;
    const double simstring_threshold;
    const size_t max_candidate;

    std::unordered_map<std::string, std::shared_ptr<ResemblaInterface>> resemblas;
    std::string primary_resembla_name;

    const std::shared_ptr<Indexer> indexer;
    const std::shared_ptr<FeatureExtractor> preprocess;
    const std::shared_ptr<ScoreFunction> score_func;
    const Reranker<string_type> reranker;

    std::unordered_map<string_type, typename FeatureExtractor::output_type> corpus_features;

    mutable std::mutex mutex_simstring;

    void load(const std::string& inverse_path)
    {
        for(const auto& columns: CsvReader<std::string>(inverse_path, 2)){
            const auto& indexed = cast_string<string_type>(columns[0]);
            const auto& original = cast_string<string_type>(columns[1]);

            auto p = inverse.insert(std::pair<string_type, std::vector<string_type>>(indexed, {original}));
            if(!p.second){
                p.first->second.push_back(original);
            }

            if(columns.size() > 2){
                const auto& features = columns[2];
#ifdef DEBUG
                std::cerr << "load from JSON: " << features << std::endl;
#endif
                nlohmann::json j = nlohmann::json::parse(features);
                typename FeatureExtractor::output_type preprocessed;
                for(nlohmann::json::iterator i = std::begin(j); i != std::end(j); ++i){
                    preprocessed[i.key()] = i.value();
                }
                corpus_features[original] = preprocessed;
            }
            else{
#ifdef DEBUG
                std::cerr << "preprocess: " << cast_string<std::string>(original) << std::endl;
#endif
                corpus_features[original] = (*preprocess)(original, "");
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

        // rerank by regression
        std::vector<ResemblaInterface::output_type> results;
        for(const auto& r: reranker.rerank(input_data, std::begin(candidates), std::end(candidates),
                *score_func, threshold, max_response)){
            results.push_back({r.first, score_func->name, std::max(std::min(r.second, 1.0), 0.0)});
        }
        return results;
    }
};

}
#endif
