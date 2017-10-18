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

#include <vector>
#include <unordered_map>
#include <memory>
#include <algorithm>

#include "resembla_interface.hpp"

namespace resembla {

template<typename Database, typename Aggregator>
class ResemblaEnsemble: public ResemblaInterface
{
public:
    ResemblaEnsemble(std::shared_ptr<Database> database,
            std::shared_ptr<Aggregator> aggregate, size_t max_candidate):
        database(database), aggregate(aggregate),
        max_candidate(max_candidate)
    {}

    void append(const std::shared_ptr<ResemblaInterface> resembla, double weight = 1.0)
    {
        children.push_back(resembla);
        weights.push_back(weight);
    }

    std::vector<output_type> find(const string_type& query,
            double threshold = 0.0, size_t max_response = 0) const
    {
        return eval(query, database->search(query,
                max_candidate == 0 ? max_candidate : std::max(max_candidate, max_response)),
                threshold, max_response);
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
            double score = (*aggregate)(weights, p.second);
            if(score >= threshold){
                results.push_back({p.first, score});
            }
        }

        if(max_response != 0 && results.size() > max_response){
            std::partial_sort(std::begin(results), std::begin(results) + max_response, std::end(results));
            results.erase(std::begin(results) + max_response, std::end(results));
        }
        else{
            std::sort(std::begin(results), std::end(results));
        }

        return results;
    }

protected:
    const std::shared_ptr<Database> database;
    const std::shared_ptr<Aggregator> aggregate;

    std::vector<std::shared_ptr<ResemblaInterface>> children;
    std::vector<double> weights;

    const size_t max_candidate;
};

}
#endif
