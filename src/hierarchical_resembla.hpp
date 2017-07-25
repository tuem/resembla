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

#ifndef __HIERARCHICAL_RESEMBLA_HPP__
#define __HIERARCHICAL_RESEMBLA_HPP__

#include <string>
#include <vector>
#include <memory>

#include <resembla/resembla_interface.hpp>
#include <resembla/reranker.hpp>

namespace resembla {

template<class Preprocessor, class ScoreFunction>
class HierarchicalResembla: public ResemblaInterface
{
public:
    HierarchicalResembla(std::shared_ptr<ResemblaInterface> resembla, size_t max_candidate,
            const std::string corpus_features_path, 
            std::shared_ptr<Preprocessor> preprocess, std::shared_ptr<ScoreFunction> score_func):
        resembla(resembla), max_candidate(max_candidate), preprocess(preprocess), score_func(score_func),
        reranker()
    {
        if(!corpus_features_path.empty()){
            // TODO: load features of corpus texts
        }
    }

    std::vector<ResemblaInterface::response_type> getSimilarTexts(
            const string_type& input, size_t max_response, double threshold)
    {
        // extract candidates using original resembla
        std::vector<WorkData> candidates;
        auto original_results = resembla->getSimilarTexts(input, max_candidate, threshold);
        for(const auto& original_result: original_results){
            candidates.push_back(std::make_pair(original_result.text, (*preprocess)(original_result)));
        }

        // rerank by its own metric
        WorkData input_data = std::make_pair(input, (*preprocess)(input));
        auto reranked = reranker.rerank(input_data, std::begin(candidates), std::end(candidates), *score_func);

        std::vector<ResemblaInterface::response_type> results;
        for(const auto& r: reranked){
            if(r.second < threshold || results.size() >= max_response){
                break;
            }
            results.push_back({r.first, score_func->name, r.second});
        }
        return results;
    }

protected:
    using WorkData = std::pair<string_type, typename Preprocessor::return_type>;

    const std::shared_ptr<ResemblaInterface> resembla;
    const size_t max_candidate;

    const std::shared_ptr<Preprocessor> preprocess;
    const std::shared_ptr<ScoreFunction> score_func;
    const Reranker<string_type> reranker;
};

}
#endif
