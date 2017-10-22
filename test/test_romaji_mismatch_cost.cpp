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

#include <iostream>

#include "Catch/catch.hpp"

#include "string_util.hpp"

#include "measure/romaji_mismatch_cost.hpp"

using namespace resembla;

void test_romaji_mismatch_cost(const wchar_t a, const wchar_t b,
        double case_mismatch_cost, double similar_letter_cost,
        double correct)
{
    init_locale();
    RomajiMismatchCost cost(case_mismatch_cost, similar_letter_cost);
    double answer = cost(a, b);
#ifdef DEBUG
    std::wcerr << L"romaji_mismatch_cost: " << L"cost('" << a << L"','" << b << L"')=" << answer << std::endl;
#endif
    CHECK(answer == Approx(correct));
}

TEST_CASE( "check romaji_mismatch_cost with typical weight settings", "[language]" ) {
    test_romaji_mismatch_cost(L'A', L'I', 0.1L, 0.2L, 1L);
    test_romaji_mismatch_cost(L'U', L'e', 0.1L, 0.2L, 1L);
    test_romaji_mismatch_cost(L'o', L'O', 0.1L, 0.2L, 0.1L);
    test_romaji_mismatch_cost(L'K', L'S', 0.1L, 0.2L, 1L);
    test_romaji_mismatch_cost(L'D', L'd', 0.1L, 0.2L, 0.1L);
    test_romaji_mismatch_cost(L'B', L'V', 0.1L, 0.2L, 0.2L);
    test_romaji_mismatch_cost(L'c', L'k', 0.1L, 0.2L, 0.2L);
    test_romaji_mismatch_cost(L'c', L'Q', 0.1L, 0.2L, 0.3L);
    test_romaji_mismatch_cost(L'K', L'q', 0.1L, 0.2L, 0.3L);
    test_romaji_mismatch_cost(L's', L'c', 0.1L, 0.2L, 0.2L);
    test_romaji_mismatch_cost(L'f', L'h', 0.1L, 0.2L, 0.2L);
    test_romaji_mismatch_cost(L'l', L'r', 0.1L, 0.2L, 0.2L);
    test_romaji_mismatch_cost(L'j', L'z', 0.1L, 0.2L, 0.2L);
    test_romaji_mismatch_cost(L'x', L'z', 0.1L, 0.2L, 0.2L);
    test_romaji_mismatch_cost(L'a', L'-', 0.1L, 0.2L, 0.2L);
    test_romaji_mismatch_cost(L'-', L'i', 0.1L, 0.2L, 0.2L);
    test_romaji_mismatch_cost(L'u', L'-', 0.1L, 0.2L, 0.2L);
    test_romaji_mismatch_cost(L'-', L'e', 0.1L, 0.2L, 0.2L);
    test_romaji_mismatch_cost(L'o', L'-', 0.1L, 0.2L, 0.2L);
}

TEST_CASE( "check romaji_mismatch_cost when input letters are the same", "[language]" ) {
    test_romaji_mismatch_cost(L'A', L'A', 0.1L, 0.2L, 0L);
    test_romaji_mismatch_cost(L'i', L'i', 0.1L, 0.2L, 0L);
    test_romaji_mismatch_cost(L'K', L'K', 0.1L, 0.2L, 0L);
    test_romaji_mismatch_cost(L'z', L'z', 0.1L, 0.2L, 0L);
    test_romaji_mismatch_cost(L'0', L'0', 0.1L, 0.2L, 0L);
    test_romaji_mismatch_cost(L'あ', L'あ', 0.1L, 0.2L, 0L);
    test_romaji_mismatch_cost(L'宛', L'宛', 0.1L, 0.2L, 0L);
}

TEST_CASE( "check romaji_mismatch_cost when case_mismatch_cost and similar_letter_cost are not so small", "[language]" ) {
    test_romaji_mismatch_cost(L'A', L'I', 0.8L, 0.9L, 1L);
    test_romaji_mismatch_cost(L'U', L'e', 0.8L, 0.9L, 1L);
    test_romaji_mismatch_cost(L'o', L'O', 0.8L, 0.9L, 0.8L);
    test_romaji_mismatch_cost(L'K', L'S', 0.8L, 0.9L, 1L);
    test_romaji_mismatch_cost(L'D', L'd', 0.8L, 0.9L, 0.8L);
    test_romaji_mismatch_cost(L'B', L'V', 0.8L, 0.9L, 0.9L);
    test_romaji_mismatch_cost(L'h', L'f', 0.8L, 0.9L, 0.9L);
    test_romaji_mismatch_cost(L'l', L'R', 0.8L, 0.9L, 1L);
    test_romaji_mismatch_cost(L'K', L'q', 0.8L, 0.9L, 1L);
    test_romaji_mismatch_cost(L'x', L'z', 0.8L, 0.9L, 0.9L);
}
