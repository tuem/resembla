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

#ifndef RESEMBLA_RESEMBLA_ENSEMBLE_HPP
#define RESEMBLA_RESEMBLA_ENSEMBLE_HPP

#include <math.h>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <algorithm>

#include <simstring/simstring.h>

#include "resembla_interface.hpp"
#include "eliminator.hpp"

namespace resembla {

template<typename Indexer, typename ScoreFunction>
class ResemblaEnsemble: public ResemblaInterface
{
public:
    ResemblaEnsemble(const std::string& db_path, const std::string& index_path,
            std::shared_ptr<Indexer> indexer, std::shared_ptr<ScoreFunction> score,
            const std::string& measure_name, const size_t max_candidate = 0):
        indexer(indexer), score(score),
        measure_name(measure_name), max_candidate(max_candidate),
        total_weight(0.0)
    {
        load(db_path, index_path);
    }

    void append(const std::shared_ptr<ResemblaInterface> resembla, double weight = 1.0)
    {
        children.push_back(std::make_pair(resembla, weight));
        total_weight += weight;
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

        return eval(query, simstring_result, threshold, max_response);
    }

    std::vector<output_type> eval(const string_type& query, const std::vector<string_type>& candidates,
            double threshold = 0.0, size_t max_response = 0) const
    {
        // calculate similarity using all measures
        std::unordered_map<string_type, double> aggregated;
        for(auto p: resemblas){
            for(auto r: p.first->eval(query, targets, 0.0, 0)){
                if(aggregated.find(r.text) == std::end(aggregated)){
                    aggregated[r.text] = 0.0;
                }
                aggregated.at(r.text) += p.second * r.score * r.score;
            }
        }

        // sort combined result
        std::vector<output_type> response;
        for(auto r: aggregated){
            double score = sqrt(r.second / total_weight);
            if(score >= threshold){
                response.push_back({r.first, measure_name, score});
            }
        }
        std::sort(std::begin(response), std::end(response));

        // return at most max_response responses
        if(max_response != 0 && response.size() > max_response){
            response.erase(std::begin(response) + max_response, std::end(response));
        }
        return response;
    }

protected:
    struct Child
    {
        std::string name;
        std::shared_ptr<ResemblaInterface> resembla;
        double weight;
    };

    // name to be used in response
    const std::string measure_name;

    const size_t max_candidate;

    double total_weight;

    std::vector<Child> children;
};

}
#endif
