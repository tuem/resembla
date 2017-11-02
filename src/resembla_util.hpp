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

#ifndef RESEMBLA_RESEMBLA_UTIL_HPP
#define RESEMBLA_RESEMBLA_UTIL_HPP

#include <string>
#include <memory>
#include <vector>

#include <paramset.hpp>

#include "simstring_database.hpp"

#include "measure/romaji_preprocessor.hpp"
#include "regression/aggregator/feature_aggregator.hpp"
#include "regression/predictor/svr_predictor.hpp"
#include "composition.hpp"

#include "basic_resembla.hpp"
#include "resembla_regression.hpp"

namespace resembla {

// for converting enums to strings
#define STR(val) #val

extern const std::string SIMSTRING_DB_FILE_COMMON_SUFFIX;
extern const std::string SIMSTRING_INVERSE_FILE_COMMON_SUFFIX;

enum measure: int
{
    edit_distance,
    weighted_word_edit_distance,
    weighted_pronunciation_edit_distance,
    weighted_romaji_edit_distance,
    keyword_match,
    ensemble,
    svr
};

// utility function for converting string that represents a simstring measure to int
int simstring_measure_from_string(const std::string& simstring_measure_str);

// utility function for generating database file path for SimString from Resembla measure
std::string db_path_from_resembla_measure(const std::string& corpus_path, const measure resembla_measure);

// utility function for generating file path of inverted index for original and parsed texts from Resembla measure
std::string inverse_path_from_resembla_measure(const std::string& corpus_path, const measure resembla_measure);

// split text by delimiter and parse to resembla measures
std::vector<measure> split_to_resembla_measures(std::string text, char delimiter = ',', bool ignore_unknown_measure = false);

// utility function for creating Resembla instance
template<typename Database, typename Preprocessor, typename ScoreFunction>
std::shared_ptr<ResemblaInterface> construct_basic_resembla(
        std::shared_ptr<Database> database, std::shared_ptr<Preprocessor> preprocess,
        std::shared_ptr<ScoreFunction> score_func,
        size_t max_candidate, const std::string& index_path)
{
    return std::make_shared<BasicResembla<Database, Preprocessor, ScoreFunction>>(
            database, preprocess, score_func,
            max_candidate, index_path);
}

std::shared_ptr<ResemblaRegression<SimStringDatabase<RomajiPreprocessor>, Composition<FeatureAggregator, SVRPredictor>>>
construct_resembla_regression(const std::string& simstring_db_path, const std::string& resembla_index_path,
        const paramset::manager& pm, const std::shared_ptr<ResemblaInterface> resembla);

// utility function to construct Resembla instance
std::shared_ptr<ResemblaInterface> construct_resembla(const paramset::manager& pm);

std::vector<std::vector<std::string>> load_features(const std::string& file_path);

}
#endif
