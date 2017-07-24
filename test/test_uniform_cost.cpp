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

#include <string>
#include <iostream>

#include "Catch/catch.hpp"

#include "string_util.hpp"

#include "measure/uniform_cost.hpp"

using namespace resembla;

void test_cost_match_func(const wchar_t a, const wchar_t b, double c)
{
    init_locale();
    UniformCost cost;
    CHECK(cost(a, b) == c);
}

TEST_CASE( "check char_match_cost with same symbols", "[language]" ) {
    test_cost_match_func(L'a', L'a', 0L);
    test_cost_match_func(L'ア', L'ア', 0L);
    test_cost_match_func(L'あ', L'あ', 0L);
    test_cost_match_func(L'亜', L'亜', 0L);
}

TEST_CASE( "check char_match_cost with different symbols", "[language]" ) {
    test_cost_match_func(L'b', L'c', 1L);
    test_cost_match_func(L'イ', L'ウ', 1L);
    test_cost_match_func(L'い', L'う', 1L);
    test_cost_match_func(L'偉', L'兎', 1L);
}
