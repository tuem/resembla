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

#ifndef RESEMBLA_BASIC_RESEMBLA_HPP
#define RESEMBLA_BASIC_RESEMBLA_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

#include <json.hpp>

#include "resembla_interface.hpp"
#include "csv_reader.hpp"
#include "reranker.hpp"

namespace resembla {

template<typename Database, typename Preprocessor, typename ScoreFunction>
class BasicResembla: public ResemblaInterface
{
public:
    BasicResembla(
            std::shared_ptr<Database> database,
            std::shared_ptr<Preprocessor> preprocess,
            std::shared_ptr<ScoreFunction> score_func,
            size_t max_candidate = 0, const std::string& index_path = ""):
        database(database), preprocess(preprocess), score_func(score_func),
        reranker(), max_candidate(max_candidate)
    {
        if(index_path.empty()){
            return;
        }

        for(const auto& columns: CsvReader<string_type>(index_path, 2)){
            const auto& original = columns[1];

            if(columns.size() > 2 && !columns[2].empty()){
                // string => JSON => preprocessed data
                typename Preprocessor::output_type preprocessed =
                        nlohmann::json::parse(cast_string<std::string>(columns[2]));
                preprocessed_corpus[original] = preprocessed;
            }
            else{
                // generate preprocessed data here
                preprocessed_corpus[original] = (*preprocess)(original, true);
            }
        }
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
        std::vector<std::pair<string_type, WorkData>> work;
        for(const auto& t: candidates){
            const auto i = preprocessed_corpus.find(t);
            if(i != std::end(preprocessed_corpus)){
                work.push_back(*i);
            }
            else{
                work.push_back(std::make_pair(
                    split(t, column_delimiter<string_type::value_type>())[0],
                    (*preprocess)(t, true)));
            }
        }

        // execute reranking
        auto input_data = std::make_pair(query, (*preprocess)(query, false));
        std::vector<output_type> response;
        for(const auto& r: reranker.rerank(input_data, std::begin(work), std::end(work),
                *score_func, threshold, max_response)){
            response.push_back({r.first, r.second});
        }
        return response;
    }

protected:
    using WorkData = typename Preprocessor::output_type;

    std::unordered_map<string_type, WorkData> preprocessed_corpus;

    const std::shared_ptr<Database> database;
    const std::shared_ptr<Preprocessor> preprocess;
    const std::shared_ptr<ScoreFunction> score_func;

    const Reranker<string_type> reranker;
    const size_t max_candidate;
};

}
#endif
