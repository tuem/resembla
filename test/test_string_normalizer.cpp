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

#include "string_normalizer.hpp"

using namespace resembla;

void test_string_normalizer_noramlize(const std::wstring& input,
        const std::wstring& correct, bool to_lower = false)
{
    init_locale();

    StringNormalizer normalize_text("../misc/icu/normalization", "resembla", "nfkc",
            "../misc/icu/transliteration/resembla_collapse.txt", to_lower);

    auto answer = normalize_text(input);
#ifdef DEBUG
    std::wcerr << "input  : " << input <<  std::endl;
    std::wcerr << "answer : " << answer << std::endl;
    std::wcerr << "correct: " << correct << std::endl;
#endif
    CHECK(cast_string<std::string>(answer) == cast_string<std::string>(correct));
}

TEST_CASE( "empty string normalizer", "[language]" ) {
    std::string input = "ＨｅLLo、＠＄％!！？！!";

    StringNormalizer normalize_nothing("", "", "", "", false);
    CHECK(normalize_nothing(input) == "ＨｅLLo、＠＄％!！？！!");

    StringNormalizer normalize_case("", "", "", "", true);
    CHECK(normalize_case(input) == "ｈｅllo、＠＄％!！？！!");
}

TEST_CASE( "truncate marks in prefix and suffix", "[language]" ) {
    test_string_normalizer_noramlize(L"", L"");

    test_string_normalizer_noramlize(L"あいうえお", L"あいうえお");
    test_string_normalizer_noramlize(L"あ?い!う?え!お", L"あ?い!う?え!お");

    test_string_normalizer_noramlize(L"!", L"");
    test_string_normalizer_noramlize(L"?", L"?");
    test_string_normalizer_noramlize(L"!?!", L"?");
    test_string_normalizer_noramlize(L"?!?", L"?");
    test_string_normalizer_noramlize(L"テスト!?", L"テスト?");
    test_string_normalizer_noramlize(L"テスト?!", L"テスト?");

    test_string_normalizer_noramlize(L"サッカーーーーーーーーーー", L"サッカー");
    test_string_normalizer_noramlize(L"〜〜〜テスト〜〜〜", L"テスト");
    test_string_normalizer_noramlize(L"渋谷〜〜新宿", L"渋谷~新宿");
    test_string_normalizer_noramlize(L"--this is a test--", L"this is a test");

    test_string_normalizer_noramlize(L"!あいうえお", L"あいうえお");
    test_string_normalizer_noramlize(L"?あいうえお", L"あいうえお");

    test_string_normalizer_noramlize(L"あいうえお!", L"あいうえお");
    test_string_normalizer_noramlize(L"あいうえお!", L"あいうえお");
    test_string_normalizer_noramlize(L"あいうえお?", L"あいうえお?");

    test_string_normalizer_noramlize(L"!?!あいうえお", L"あいうえお");
    test_string_normalizer_noramlize(L"あいうえお?!?!!", L"あいうえお?");
    test_string_normalizer_noramlize(L"あいうえお!?!!?", L"あいうえお?");
    test_string_normalizer_noramlize(L"?!?あ!い!う?え?お!?!", L"あ!い!う?え?お?");

    test_string_normalizer_noramlize(L" ", L"");
    test_string_normalizer_noramlize(L"a ", L"a");
    test_string_normalizer_noramlize(L" a", L"a");
    test_string_normalizer_noramlize(L" a b ", L"a b");
    test_string_normalizer_noramlize(L"  ab  cd  ", L"ab cd");
    test_string_normalizer_noramlize(L"  a b  c  d   e   f  ", L"a b c d e f");
    test_string_normalizer_noramlize(L"  a B  c  Ｄ   e   F  ", L"a b c d e f", true);

    test_string_normalizer_noramlize(L"テ。ス、ト。", L"テ ス ト");
    test_string_normalizer_noramlize(L"、、テ。。。ス、、、、ト。。。。。", L"テ ス ト");
    test_string_normalizer_noramlize(L"、。、テ。。。ス、、、、ト。、。。。、。", L"テ ス ト");
}
