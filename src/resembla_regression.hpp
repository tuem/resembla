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

#ifndef __RESEMBLA_REGRESSION_HPP__
#define __RESEMBLA_REGRESSION_HPP__

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <fstream>

#include <resembla/resembla_interface.hpp>
#include <resembla/reranker.hpp>

namespace resembla {

template<class Preprocessor, class ScoreFunction>
class ResemblaRegression: public ResemblaInterface
{
public:
    ResemblaRegression(size_t max_candidate,
            std::shared_ptr<Preprocessor> preprocess, std::shared_ptr<ScoreFunction> score_func,
            std::string corpus_path = "", size_t col = 2):
        max_candidate(max_candidate),
        preprocess(preprocess), score_func(score_func), reranker(), preprocess_corpus(!corpus_path.empty())
    {
        if(preprocess_corpus){
            loadCorpusFeatures(corpus_path, col);
        }
    }

    void append(const std::string name, const std::shared_ptr<ResemblaInterface> resembla, bool is_primary)
    {
        resemblas[name] = resembla;
        if(is_primary){
            primary_resembla_name = name;
        }
    }

    std::vector<response_type> getSimilarTexts(const string_type& input, size_t max_response, double threshold)
    {
        std::vector<string_type> candidate_texts;
        std::unordered_map<string_type, StringFeatureMap> aggregated;

        // primary resembla
        auto initial_results = resemblas[primary_resembla_name]->getSimilarTexts(input, max_candidate, threshold);
        for(auto c: initial_results){
            candidate_texts.push_back(c.text);
            aggregated[c.text] = preprocess_corpus ?
                    corpus_features[c.text] : ((*preprocess)(c.text));
            aggregated[c.text][primary_resembla_name] = Feature::toText(c.score);
        }

        // other resemblas
        for(auto p: resemblas){
            if(p.first == primary_resembla_name){
                continue;
            }
            for(auto r: p.second->getSimilarTexts(input, candidate_texts)){
                if(aggregated.find(r.text) == aggregated.end()){
                    aggregated[r.text] = preprocess_corpus ?
                            corpus_features[r.text] : ((*preprocess)(r.text));
                }
                aggregated[r.text][p.first] = Feature::toText(r.score);
            }
        }

        std::vector<std::pair<string_type, StringFeatureMap>> candidates;
        for(auto a: aggregated){
            candidates.push_back(std::make_pair(a.first, a.second));
        }

        // rerank by its own metric
        std::pair<string_type, StringFeatureMap> input_data = std::make_pair(input, (*preprocess)(input));
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

    std::vector<response_type> getSimilarTexts(const string_type& query, const std::vector<string_type>& targets)
    {
        return resemblas[primary_resembla_name]->getSimilarTexts(query, targets);
        /*TODO
        std::vector<WorkData> candidates;
        auto original_results = resembla->getSimilarTexts(query, targets);
        for(const auto& r: original_results){
            candidates.push_back(std::make_pair(r.text, (*preprocess)(r,
                    preprocess_corpus ? corpus_features[r.text] : decltype((*preprocess)(r.text))())));
        }

        // rerank by its own metric
        WorkData query_data = std::make_pair(query, (*preprocess)(query));
        auto reranked = reranker.rerank(query_data, std::begin(candidates), std::end(candidates), *score_func);

        std::vector<ResemblaInterface::response_type> results;
        for(const auto& r: reranked){
            results.push_back({r.first, score_func->name, r.second});
        }
        return results;
        */
    }

protected:
    struct WorkData
    {
        string_type text;
        typename Preprocessor::return_type features;
    };
    //using WorkData = std::pair<string_type, typename Preprocessor::return_type>;

    std::unordered_map<std::string, std::shared_ptr<ResemblaInterface>> resemblas;
    //const std::shared_ptr<ResemblaInterface> resembla;
    std::string primary_resembla_name;
    const size_t max_candidate;

    const std::shared_ptr<Preprocessor> preprocess;
    const std::shared_ptr<ScoreFunction> score_func;
    const Reranker<string_type> reranker;

    const bool preprocess_corpus;
    std::unordered_map<string_type, typename Preprocessor::return_type> corpus_features;

    void loadCorpusFeatures(const std::string& corpus_path, size_t col)
    {
        std::ifstream ifs(corpus_path);
        if(ifs.fail()){
            throw std::runtime_error("input file is not available: " + corpus_path);
        }
        while(ifs.good()){
            std::string line;
            std::getline(ifs, line);
            if(ifs.eof() || line.length() == 0){
                break;
            }
            auto columns = split(line, '\t');
            if(columns.size() + 1 < col){
                continue;
            }
            corpus_features[cast_string<string_type>(columns[0])] = (*preprocess)(columns[0], columns[1]);
        }
    }
};

}
#endif
