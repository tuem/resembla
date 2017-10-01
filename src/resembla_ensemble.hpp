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

#ifndef RESEMBLA_RESEMBLA_ENSEMBLE_HPP
#define RESEMBLA_RESEMBLA_ENSEMBLE_HPP

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <algorithm>

#include <simstring/simstring.h>

#include "resembla_interface.hpp"
#include "csv_reader.hpp"
#include "eliminator.hpp"

namespace resembla {

template<typename Indexer, typename ScoreFunction>
class ResemblaEnsemble: public ResemblaInterface
{
public:
    ResemblaEnsemble(const std::string& measure_name,
            const std::string& simstring_db_path, const std::string& index_path,
            const int simstring_measure, const double simstring_threshold, const size_t max_candidate,
            std::shared_ptr<Indexer> indexer, std::shared_ptr<ScoreFunction> score_func):
        measure_name(measure_name), simstring_measure(simstring_measure), simstring_threshold(simstring_threshold),
        max_candidate(max_candidate), indexer(indexer), score_func(score_func)
    {
        load(simstring_db_path, index_path);
    }

    void append(const std::shared_ptr<ResemblaInterface> resembla, double weight = 1.0)
    {
        children.push_back(resembla);
        weights.push_back(weight);
    }

    std::vector<output_type> find(const string_type& query,
            double threshold = 0.0, size_t max_response = 0) const
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
        std::vector<string_type> candidates;
        for(const auto& i: simstring_result){
            if(i.empty()){
                continue;
            }
            const auto& j = inverse.at(i);
            std::copy(std::begin(j), std::end(j), std::back_inserter(candidates));
        }

        return eval(query, candidates, threshold, max_response);
    }

    std::vector<output_type> eval(const string_type& query, const std::vector<string_type>& candidates,
            double threshold = 0.0, size_t max_response = 0) const
    {
        std::unordered_map<string_type, std::vector<double>> work;
        for(const auto& resembla: children){
            for(const auto& r: resembla->eval(query, candidates, 0.0, 0)){
                auto p = work.insert(std::pair<string_type, std::vector<double>>(r.text, {r.score}));
                if(!p.second){
                    p.first->second.push_back(r.score);
                }
            }
        }

        std::vector<output_type> results;
        for(const auto& p: work){
            double score = (*score_func)(weights, p.second);
            if(score >= threshold){
                results.push_back({p.first, measure_name, score});
            }
        }

        if(max_response != 0 && results.size() > max_response){
            std::partial_sort(results.begin(), results.begin() + max_response, results.end());
            results.erase(results.begin() + max_response, results.end());
        }
        else{
            std::sort(results.begin(), results.end());
        }

        return results;
    }

protected:
    const std::string measure_name;

    mutable simstring::reader db;
    mutable std::mutex mutex_simstring;
    const int simstring_measure;
    const double simstring_threshold;

    std::unordered_map<string_type, std::vector<string_type>> inverse;

    const size_t max_candidate;

    const std::shared_ptr<Indexer> indexer;

    std::vector<std::shared_ptr<ResemblaInterface>> children;
    std::vector<double> weights;

    const std::shared_ptr<ScoreFunction> score_func;

    void load(const std::string& simstring_db_path, const std::string& index_path)
    {
        db.open(simstring_db_path);

        for(const auto& columns: CsvReader<string_type>(index_path, 2)){
            const auto& indexed = columns[0];
            const auto& original = columns[1];

            auto p = inverse.insert(std::pair<string_type, std::vector<string_type>>(indexed, {original}));
            if(!p.second){
                p.first->second.push_back(original);
            }
        }
    }
};

}
#endif
