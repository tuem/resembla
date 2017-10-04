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
#include "eliminator.hpp"
#include "reranker.hpp"

namespace resembla {

template<typename Database, typename Preprocessor, typename ScoreFunction>
class BasicResembla: public ResemblaInterface
{
public:
    BasicResembla(std::shared_ptr<Database> database,
            std::shared_ptr<Preprocessor> preprocess, std::shared_ptr<ScoreFunction> score_func,
            size_t max_candidate, const std::string& inverse_path,
            bool preprocess_corpus = true, size_t preprocessed_data_col = 3):
        database(database), preprocess(preprocess), score_func(score_func),
        max_candidate(max_candidate), preprocess_corpus(preprocess_corpus)
    {
        if(!preprocess_corpus){
            return;
        }

        for(const auto& columns: CsvReader<string_type>(inverse_path, 3)){
            const auto& original = columns[1];

            if(preprocessed_data_col > 0 && preprocessed_data_col - 1 < columns.size() &&
                    !columns[preprocessed_data_col - 1].empty()){
                nlohmann::json j = nlohmann::json::parse(
                        cast_string<std::string>(columns[preprocessed_data_col - 1]));
                typename Preprocessor::output_type preprocessed = j;
                preprocessed_corpus[original] = std::make_pair(original, preprocessed);
            }
            else{
                preprocessed_corpus[original] = std::make_pair(original, (*preprocess)(original, true));
            }
        }
    }

    std::vector<output_type> find(const string_type& query,
            double threshold = 0.0, size_t max_response = 0) const
    {
        if(database == nullptr){
            return {};
        }

        return eval(query, database->search(query, max_candidate), threshold, max_response);
    }

    std::vector<output_type> eval(const string_type& query, const std::vector<string_type>& candidates,
            double threshold = 0.0, size_t max_response = 0) const
    {
        std::vector<WorkData> work;
        for(const auto& t: candidates){
            if(preprocess_corpus){
                const auto i = preprocessed_corpus.find(t);
                if(i != std::end(preprocessed_corpus)){
                    work.push_back(i->second);
                    continue;
                }
            }
            auto tabpos = t.find(column_delimiter<string_type::value_type>());
            work.push_back(std::make_pair(
                tabpos != string_type::npos ? t.substr(0, tabpos) : t,
                (*preprocess)(t, true)));
        }

        // execute reranking
        auto input_data = std::make_pair(query, (*preprocess)(query, false));
        std::vector<output_type> response;
        for(const auto& r: reranker.rerank(input_data, work.begin(), work.end(),
                *score_func, threshold, max_response)){
            response.push_back({r.first, score_func->name, r.second});
        }
        return response;
    }

protected:
    using WorkData = std::pair<string_type, typename Preprocessor::output_type>;

    const std::shared_ptr<Database> database;
    const std::shared_ptr<Preprocessor> preprocess;
    const std::shared_ptr<ScoreFunction> score_func;

    const Reranker<string_type> reranker;
    const size_t max_candidate;

    std::unordered_map<string_type, WorkData> preprocessed_corpus;
    const bool preprocess_corpus;
};

}
#endif
