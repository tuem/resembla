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

#ifndef __BOUNDED_RESEMBLA_HPP__
#define __BOUNDED_RESEMBLA_HPP__

#include <string>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <memory>
#include <stdexcept>

#include "simstring/simstring.h"

#include "resembla_interface.hpp"
#include "reranker.hpp"

namespace resembla {

// Resembla with a fixed pair of preprocessor and score function
template<
    typename Preprocessor,
    typename ScoreFunction
>
class BoundedResembla: public ResemblaInterface
{
public:
    BoundedResembla(const std::string& db_path, const std::string& inverse_path,
            const int simstring_measure, const double simstring_threshold, const size_t max_reranking_num,
            std::shared_ptr<Preprocessor> preprocess, std::shared_ptr<ScoreFunction> score_func, bool preprocess_corpus = true):
        simstring_measure(simstring_measure), simstring_threshold(simstring_threshold), max_reranking_num(max_reranking_num),
        reranker(), preprocess(preprocess), score_func(score_func), preprocess_corpus(preprocess_corpus)
    {
        db.open(db_path);
        std::basic_ifstream<string_type::value_type> ifs(inverse_path);
        if(ifs.fail()){
            throw std::runtime_error("input file is not available: " + inverse_path);
        }
        while(ifs.good()){
            string_type line;
            std::getline(ifs, line);
            if(ifs.eof() || line.length() == 0){
                break;
            }
            size_t i = line.find(L"\t");
            string_type indexed = line.substr(0, i);
            string_type original = line.substr(i + 1);
            if(inverse.count(indexed) == 0){
                inverse[indexed] = {original};
            }
            else{
                inverse[indexed].push_back(original);
            }
            if(preprocess_corpus){
                preprocessed_corpus[original] = std::make_pair(original, (*preprocess)(original, true));
            }
        }
    }

    std::vector<response_type> getSimilarTexts(const string_type& query, size_t max_response, double threshold)
    {
        // search from N-gram index
        string_type search_query = preprocess->buildIndexingText(query);
        std::vector<string_type> simstring_result;
        db.retrieve(search_query, simstring_measure, simstring_threshold, std::back_inserter(simstring_result));
        if(simstring_result.empty()){
            return {};
        }

        // load preprocessed data if preprocessing is enabled. otherwise, process corpus texts on demand
        std::vector<WorkData> candidates;
        for(const auto& i: simstring_result){
            for(const string_type& t: inverse[i]){
                candidates.push_back(preprocess_corpus ? preprocessed_corpus[t] : std::make_pair(t, (*preprocess)(t, true)));
                if(candidates.size() == max_reranking_num){
                    break;
                }
            }
        }

        // execute reranking
        WorkData input_data = std::make_pair(query, (*preprocess)(query, false));
        std::vector<response_type> response;
        for(const auto& r: reranker.rerank(input_data, std::begin(candidates), std::end(candidates), *score_func)){
            if(r.second < threshold || response.size() == max_response){
                break;
            }
            response.push_back({r.first, score_func->name, r.second});
        }
        return response;
    }

    std::vector<response_type> calcSimilarity(const string_type& query, const std::vector<string_type>& targets)
    {
        // load preprocessed data if preprocessing is enabled. otherwise, process corpus texts on demand
        std::vector<WorkData> candidates;
        for(const auto& t: targets){
            candidates.push_back(preprocess_corpus ? preprocessed_corpus[t] : std::make_pair(t, (*preprocess)(t, true)));
        }

        // execute reranking
        WorkData input_data = std::make_pair(query, (*preprocess)(query, false));
        std::vector<response_type> response;
        for(const auto& r: reranker.rerank(input_data, std::begin(candidates), std::end(candidates), *score_func)){
            response.push_back({r.first, score_func->name, r.second});
        }
        return response;
    }

protected:
    using WorkData = std::pair<string_type, typename Preprocessor::output_type>;

    simstring::reader db;
    std::unordered_map<string_type, std::vector<string_type>> inverse;

    const int simstring_measure;
    const double simstring_threshold;
    const size_t max_reranking_num;
    const Reranker<string_type> reranker;

    const std::shared_ptr<Preprocessor> preprocess;
    const std::shared_ptr<ScoreFunction> score_func;

    const bool preprocess_corpus;
    std::unordered_map<string_type, WorkData> preprocessed_corpus;
};

}
#endif
