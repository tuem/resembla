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

#include "measure/romaji_weight.hpp"

using namespace resembla;

void test_romaji_weight(const wchar_t a, double base_weight, double delete_insert_ratio, 
        double uppercase_coefficient, double lowercase_coefficient,
        double vowel_coefficient, double consonant_coefficient,
        bool is_original, double correct)
{
    init_locale();
    RomajiWeight weight(base_weight, delete_insert_ratio,
            uppercase_coefficient, lowercase_coefficient,
            vowel_coefficient, consonant_coefficient);
    double answer = weight(a, is_original);
#ifdef DEBUG
    std::wcerr << L"letter=" << a << L", op=" << (is_original ? L"del" : L"ins") << L", weight=" << answer << std::endl;
#endif
    CHECK(answer == correct);
}

TEST_CASE( "check romaji_weight with typical weight settings", "[language]" ) {
    test_romaji_weight(L'A', 1L, 1.2L, 1L, 0.5L, 1L, 0.8L, true, 1.2L);
    test_romaji_weight(L'i', 1L, 1.2L, 1L, 0.5L, 1L, 0.8L, false, 0.5L);
    test_romaji_weight(L'K', 1L, 1.2L, 1L, 0.5L, 1L, 0.8L, false, 0.8L);
    test_romaji_weight(L'z', 1L, 1.2L, 1L, 0.5L, 1L, 0.8L, true, 0.48L);
    test_romaji_weight(L'0', 1L, 1.2L, 1L, 0.5L, 1L, 0.8L, true, 1.2L);
}

TEST_CASE( "check romaji_weight with some base_weights and delete_insert_ratios", "[language]" ) {
    test_romaji_weight(L'A', 1L, 1L, 1L, 1L, 1L, 1L, true, 1L);
    test_romaji_weight(L'i', 2L, 1L, 1L, 1L, 1L, 1L, false, 2L);
    test_romaji_weight(L'K', 3L, 10L, 1L, 1L, 1L, 1L, false, 3L);
    test_romaji_weight(L's', 4L, 10L, 1L, 1L, 1L, 1L, true, 40L);
    test_romaji_weight(L'あ', 5L, 100L, 1L, 1L, 1L, 1L, true, 500L);
    test_romaji_weight(L'宛', 6L, 100L, 1L, 1L, 1L, 1L, false, 6L);
}

TEST_CASE( "check romaji_weight with letter case weight", "[language]" ) {
    test_romaji_weight(L'A', 1L, 1L, 10L, 0.1L, 1L, 1L, true, 10L);
    test_romaji_weight(L'i', 1L, 1L, 10L, 0.1L, 1L, 1L, false, 0.1L);
    test_romaji_weight(L'K', 1L, 1L, 10L, 0.1L, 1L, 1L, false, 10L);
    test_romaji_weight(L's', 1L, 1L, 10L, 0.1L, 1L, 1L, true, 0.1L);
    test_romaji_weight(L'0', 1L, 1L, 10L, 0.1L, 1L, 1L, true, 1L);
    test_romaji_weight(L'あ', 1L, 1L, 10L, 0.1L, 1L, 1L, true, 1L);
    test_romaji_weight(L'宛', 1L, 1L, 10L, 0.1L, 1L, 1L, false, 1L);
}

TEST_CASE( "check romaji_weight with vowel/consonant weight", "[language]" ) {
    test_romaji_weight(L'A', 1L, 1L, 1L, 1L, 10L, 0.1L, true, 10L);
    test_romaji_weight(L'I', 1L, 1L, 1L, 1L, 10L, 0.1L, true, 10L);
    test_romaji_weight(L'U', 1L, 1L, 1L, 1L, 10L, 0.1L, true, 10L);
    test_romaji_weight(L'E', 1L, 1L, 1L, 1L, 10L, 0.1L, true, 10L);
    test_romaji_weight(L'O', 1L, 1L, 1L, 1L, 10L, 0.1L, true, 10L);
    test_romaji_weight(L'a', 1L, 1L, 1L, 1L, 10L, 0.1L, true, 10L);
    test_romaji_weight(L'i', 1L, 1L, 1L, 1L, 10L, 0.1L, true, 10L);
    test_romaji_weight(L'u', 1L, 1L, 1L, 1L, 10L, 0.1L, true, 10L);
    test_romaji_weight(L'e', 1L, 1L, 1L, 1L, 10L, 0.1L, true, 10L);
    test_romaji_weight(L'o', 1L, 1L, 1L, 1L, 10L, 0.1L, true, 10L);
    test_romaji_weight(L'-', 1L, 1L, 1L, 1L, 10L, 0.1L, true, 10L);
    test_romaji_weight(L'K', 1L, 1L, 1L, 1L, 10L, 0.1L, true, 0.1L);
    test_romaji_weight(L'z', 1L, 1L, 1L, 1L, 10L, 0.1L, true, 0.1L);
    test_romaji_weight(L'q', 1L, 1L, 1L, 1L, 10L, 0.1L, true, 0.1L);
    test_romaji_weight(L'0', 1L, 1L, 1L, 1L, 10L, 0.1L, true, 1L);
    test_romaji_weight(L'あ', 1L, 1L, 1L, 1L, 10L, 0.1L, true, 1L);
    test_romaji_weight(L'宛', 1L, 1L, 1L, 1L, 10L, 0.1L, true, 1L);
}

TEST_CASE( "check romaji_weight with complex weight settings", "[language]" ) {
    test_romaji_weight(L'U', 2L, 3L, 5L, 7L, 11L, 13L, true, 330L);
    test_romaji_weight(L'E', 2L, 3L, 5L, 7L, 11L, 13L, false, 110L);
    test_romaji_weight(L'i', 2L, 3L, 5L, 7L, 11L, 13L, true, 462L);
    test_romaji_weight(L'o', 2L, 3L, 5L, 7L, 11L, 13L, false, 154L);
    test_romaji_weight(L'Z', 2L, 3L, 5L, 7L, 11L, 13L, true, 390L);
    test_romaji_weight(L'P', 2L, 3L, 5L, 7L, 11L, 13L, false, 130L);
    test_romaji_weight(L'r', 2L, 3L, 5L, 7L, 11L, 13L, true, 546L);
    test_romaji_weight(L'm', 2L, 3L, 5L, 7L, 11L, 13L, false, 182L);
    test_romaji_weight(L'0', 2L, 3L, 5L, 7L, 11L, 13L, true, 6L);
    test_romaji_weight(L'あ', 2L, 3L, 5L, 7L, 11L, 13L, false, 2L);
    test_romaji_weight(L'宛', 2L, 3L, 5L, 7L, 11L, 13L, true, 6L);
}
