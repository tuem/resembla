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

#include "resembla_ensemble.hpp"

#include <math.h>
#include <algorithm>

#include "resembla_util.hpp"

namespace resembla {

ResemblaEnsemble::ResemblaEnsemble(const std::string& measure_name, const size_t max_reranking_num):
    measure_name(measure_name), max_reranking_num(max_reranking_num), total_weight(0.0) {}

void ResemblaEnsemble::append(const std::shared_ptr<ResemblaInterface> resembla, const double weight)
{
    resemblas.push_back(std::make_pair(resembla, weight));
    total_weight += weight;
}

std::vector<ResemblaEnsemble::output_type> ResemblaEnsemble::find(const string_type& query,
        double threshold, size_t max_response) const
{
    double t = max_reranking_num > 0 ? 0.0 : threshold;
    size_t n = max_reranking_num > 0 ? max_reranking_num : max_response;

    // find similar texts using all measures
    std::unordered_map<string_type, double> aggregated;
    for(auto p: resemblas){
        for(auto r: p.first->find(query, t, n)){
            if(aggregated.find(r.text) == std::end(aggregated)){
                aggregated[r.text] = 0.0;
            }
            aggregated.at(r.text) += p.second * r.score * r.score;
        }
    }

    return eval(aggregated, threshold, max_response);
}

std::vector<ResemblaInterface::output_type> ResemblaEnsemble::eval(const string_type& query,
        const std::vector<string_type>& targets, double threshold, size_t max_response) const
{
    double t = max_reranking_num > 0 ? 0.0 : threshold;
    size_t n = max_reranking_num > 0 ? max_reranking_num : max_response;

    // calculate similarity using all measures
    std::unordered_map<string_type, double> aggregated;
    for(auto p: resemblas){
        for(auto r: p.first->eval(query, targets, t, n)){
            if(aggregated.find(r.text) == std::end(aggregated)){
                aggregated[r.text] = 0.0;
            }
            aggregated.at(r.text) += p.second * r.score * r.score;
        }
    }

    return eval(aggregated, threshold, max_response);
}

std::vector<ResemblaInterface::output_type> ResemblaEnsemble::eval(
        const std::unordered_map<string_type, double>& aggregated, double threshold, size_t max_response) const
{
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

}
