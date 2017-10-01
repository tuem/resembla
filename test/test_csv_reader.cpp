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

#include <string>
#include <iostream>

#include "Catch/catch.hpp"

#include "csv_reader.hpp"

using namespace resembla;

TEST_CASE( "read csv", "[file]" ) {
    init_locale();

    std::string file_path = "./3x3.tsv";
    size_t min_columns = 3;

    std::vector<std::vector<std::wstring>> answer;
    for(const auto& columns: CsvReader<std::wstring>(file_path, min_columns)){
        answer.push_back(columns);
    }

    std::vector<std::vector<std::wstring>> correct = {
        {L"v00", L"v01", L"v02"},
        {L"v10", L"v11", L"v12"},
        {L"v20", L"v21", L"v22"},
    };

    CHECK(answer == correct);
}
