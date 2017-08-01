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

#ifndef __RESEMBLA_UTIL_HPP__
#define __RESEMBLA_UTIL_HPP__

#include <string>
#include <memory>

#include "paramset.hpp"

#include "resembla_util.hpp"

#include "bounded_resembla.hpp"
#include "resembla_ensemble.hpp"

#include "regression/aggregator/feature_aggregator.hpp"
#include "regression/predictor/svr_predictor.hpp"
#include "composition.hpp"
#include "resembla_regression.hpp"

namespace resembla {

// for converting enums to strings
#define STR(val) #val

extern const std::string SIMSTRING_DB_FILE_SUFFIX;
extern const std::string SIMSTRING_DB_FILE_COMMON_SUFFIX;
extern const std::string SIMSTRING_INVERSE_FILE_COMMON_SUFFIX;

enum text_preprocess: int
{
    asis,
    pronunciation,
    romaji,
    keyword
};

enum measure: int
{
    edit_distance,
    weighted_word_edit_distance,
    weighted_pronunciation_edit_distance,
    weighted_romaji_edit_distance,
    keyword_match,
    svr
};

// utility function for converting string that represents a simstring measure to int
int simstring_measure_from_string(const std::string& simstring_measure_str);

// utility function for trying to read value from JSON and unnamed command line option
template<typename T> T read_value_with_rest(paramset::manager& pm, const std::string key, T throw_if)
{
    T value = pm[key];
    if(pm.rest.size() > 0){
        value = pm.rest[0].as<T>();
    }
    if(value == throw_if){
        throw std::invalid_argument("no input corpus specified");
    }
    return value;
}

// read pm[key] and overwrite with pm.rest if exists
std::string read_value_with_rest(paramset::manager& pm, const std::string key, const char* throw_if);

// utility function for generating database file path for SimString from text preprocessing method
std::string db_path_from_simstring_text_preprocess(const std::string& corpus_path, const text_preprocess simstring_text_preprocess);

// utility function for generating file path of inverted index for original and parsed texts
std::string inverse_path_from_simstring_text_preprocess(const std::string& corpus_path, const text_preprocess simstring_text_preprocess);

// utility function for generating database file path for SimString from Resembla measure
std::string db_path_from_resembla_measure(const std::string& corpus_path, const measure resembla_measure);

// utility function for generating file path of inverted index for original and parsed texts from Resembla measure
std::string inverse_path_from_resembla_measure(const std::string& corpus_path, const measure resembla_measure);

// split text by delimiter and parse to text preprocessing methods for n-gram indexes
std::vector<text_preprocess> split_to_text_preprocesses(std::string text, char delimiter = ',', bool ignore_unknown_measure = false);

// split text by delimiter and parse to resembla measures
std::vector<measure> split_to_resembla_measures(std::string text, char delimiter = ',', bool ignore_unknown_measure = false);

// utility function for creating Resembla instance
template<
    typename Preprocessor,
    typename ScoreFunction
>
std::shared_ptr<ResemblaInterface> construct_bounded_resembla(const std::string& db_path, const std::string& inverse_path,
        int simstring_measure, double simstring_threshold, int max_reranking_num,
        std::shared_ptr<Preprocessor> preprocess, std::shared_ptr<ScoreFunction> score_func,
        bool preprocess_corpus = true, size_t feature_col = 2)
{
    return std::make_shared<BoundedResembla<Preprocessor, ScoreFunction>>(
            db_path, inverse_path, simstring_measure, simstring_threshold, max_reranking_num,
            preprocess, score_func, preprocess_corpus, feature_col);
}

std::shared_ptr<ResemblaRegression<Composition<FeatureAggregator, SVRPredictor>>> construct_resembla_regression(
        int max_candidate, std::string corpus_path, int text_col, int features_col,
        std::string features_path, std::string patterns_home, std::string model_path,
        const std::shared_ptr<ResemblaInterface> resembla);

// utility function to construct Resembla instance
std::shared_ptr<ResemblaInterface> construct_resembla(std::string corpus_path, paramset::manager& pm);

std::vector<std::vector<std::string>> load_features(const std::string file_path);

}
#endif
