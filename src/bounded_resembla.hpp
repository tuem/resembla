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
#include <mutex>

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
            std::shared_ptr<Preprocessor> preprocess, std::shared_ptr<ScoreFunction> score_func,
            bool preprocess_corpus = true, size_t extra_col = 0):
        simstring_measure(simstring_measure), simstring_threshold(simstring_threshold), max_reranking_num(max_reranking_num),
        reranker(), preprocess(preprocess), score_func(score_func), preprocess_corpus(preprocess_corpus)
    {
        db.open(db_path);
        std::basic_ifstream<string_type::value_type> ifs(inverse_path);
        if(ifs.fail()){
            throw std::runtime_error("input file is not available: " + inverse_path);
        }
        auto delimiter = cast_string<string_type>(std::string("\t"));
        while(ifs.good()){
            string_type line;
            std::getline(ifs, line);
            if(ifs.eof() || line.length() == 0){
                break;
            }

            auto columns = split(line, L'\t');
            if(columns.size() < 2){
                throw std::runtime_error("invalid line in " + inverse_path + ": " + cast_string<std::string>(line));
            }
            const auto& indexed = columns[0];
            auto original = columns[1];

            // due to the limited interface of preprocessors, concatenate extra data to the original text
            if(extra_col > 0 && extra_col - 1 < columns.size() && !columns[extra_col - 1].empty()){
                original += delimiter + columns[extra_col - 1];
            }

            if(inverse.count(indexed) == 0){
                inverse[indexed] = {original};
            }
            else{
                inverse[indexed].push_back(original);
            }

            if(preprocess_corpus){
                preprocessed_corpus[columns[1]] = std::make_pair(columns[1], (*preprocess)(original, true));
            }
        }
    }

    std::vector<output_type> find(const string_type& query, double threshold = 0.0, size_t max_response = 0) const
    {
        // search from N-gram index
        std::vector<string_type> simstring_result;
        {
            std::lock_guard<std::mutex> lock(mutex_simstring);
            string_type search_query = preprocess->index(query);
            db.retrieve(search_query, simstring_measure, simstring_threshold, std::back_inserter(simstring_result));
            if(simstring_result.empty()){
                return {};
            }
        }

        std::vector<string_type> candidate_texts;
        for(const auto& i: simstring_result){
            const auto& j = inverse.at(i);
            std::copy(std::begin(j), std::end(j), std::back_inserter(candidate_texts));
            if(candidate_texts.size() == max_reranking_num){
                break;
            }
        }
        return eval(query, candidate_texts, threshold, max_response);
    }

    std::vector<output_type> eval(const string_type& query, const std::vector<string_type>& targets,
            double threshold = 0.0, size_t max_response = 0) const
    {
        // load preprocessed data if preprocessing is enabled. otherwise, process corpus texts on demand
        std::vector<WorkData> candidates;
        for(const auto& t: targets){
            if(preprocess_corpus){
                const auto i = preprocessed_corpus.find(t);
                if(i != std::end(preprocessed_corpus)){
                    candidates.push_back(i->second);
                    continue;
                }
            }
            auto tabpos = t.find(L'\t');
            candidates.push_back(std::make_pair(
                tabpos != string_type::npos ? t.substr(0, tabpos) : t,
                (*preprocess)(t, true)));
        }

        // execute reranking
        WorkData input_data = std::make_pair(query, (*preprocess)(query, false));
        std::vector<output_type> response;
        for(const auto& r: reranker.rerank(input_data, std::begin(candidates), std::end(candidates), *score_func, threshold, max_response)){
            response.push_back({r.first, score_func->name, r.second});
        }
        return response;
    }

protected:
    using WorkData = std::pair<string_type, typename Preprocessor::output_type>;

    mutable simstring::reader db;
    std::unordered_map<string_type, std::vector<string_type>> inverse;

    const int simstring_measure;
    const double simstring_threshold;
    const size_t max_reranking_num;
    const Reranker<string_type> reranker;

    const std::shared_ptr<Preprocessor> preprocess;
    const std::shared_ptr<ScoreFunction> score_func;

    const bool preprocess_corpus;
    std::unordered_map<string_type, WorkData> preprocessed_corpus;

    mutable std::mutex mutex_simstring;
};

}
#endif
