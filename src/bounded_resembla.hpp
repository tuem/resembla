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
#include <stdexcept>

#include "simstring/simstring.h"

#include "resembla_interface.hpp"
#include "reranker.hpp"

namespace resembla {

// Resembla with a fixed pair of sequence builder and distance function
template<
    typename SequenceBuilder,
    typename DistanceFunction
>
class BoundedResembla: public ResemblaInterface
{
public:
    using sequence_type = typename SequenceBuilder::sequence_type;

    BoundedResembla(const std::string& db_path, const std::string& inverse_path,
            const int simstring_measure, const double simstring_threshold, const size_t max_reranking_num,
            SequenceBuilder builder, DistanceFunction dist_func, bool preprocess_corpus = true):
        simstring_measure(simstring_measure), simstring_threshold(simstring_threshold), max_reranking_num(max_reranking_num),
        builder(builder), dist_func(dist_func), preprocess_corpus(preprocess_corpus)
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
                preprocessed_corpus[original] = std::make_pair(original, builder.build(original, true));
            }
        }
    }

    std::vector<response_type> getSimilarTexts(const string_type& query, size_t max_response = 20, double threshold = 0L)
    {
        // search from N-gram index
        string_type search_query = builder.buildIndexingText(query);
        std::vector<string_type> simstring_result;
        db.retrieve(search_query, simstring_measure, simstring_threshold, std::back_inserter(simstring_result));
        if(simstring_result.empty()){
            return {};
        }

        // load preprocessed data if preprocessing is enabled. otherwise, process corpus texts on demand
        std::vector<std::pair<string_type, sequence_type>> candidates;
        for(size_t i = 0; i < simstring_result.size() && candidates.size() < max_reranking_num; ++i){
            for(const string_type& t: inverse[simstring_result[i]]){
                candidates.push_back(preprocess_corpus ? preprocessed_corpus[t] : std::make_pair(t, builder.build(t, true)));
            }
        }

        // execute reranking
        auto reranked = reranker.rerank(builder.build(query, false), std::begin(candidates), std::end(candidates), dist_func);

        // return at most max_response texts those scores are greater than or equal to threshold
        std::vector<response_type> response;
        for(auto i = std::begin(reranked); i != std::end(reranked) && 1L - i->second >= threshold && response.size() < max_response; ++i){
            response.push_back({i->first, dist_func.name, 1L - i->second});
        }
        return response;
    }

protected:
    simstring::reader db;
    std::unordered_map<string_type, std::vector<string_type>> inverse;

    int simstring_measure;
    double simstring_threshold;
    size_t max_reranking_num;
    Reranker<string_type> reranker;

    SequenceBuilder builder;
    DistanceFunction dist_func;

    bool preprocess_corpus;
    std::unordered_map<string_type, std::pair<string_type, sequence_type>> preprocessed_corpus;
};

}
#endif
