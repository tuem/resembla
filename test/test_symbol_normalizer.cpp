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

#include "string_util.hpp"

#include "symbol_normalizer.hpp"

using namespace resembla;

void test_symbol_normalizer_noramlize_symbol(const std::wstring& input, const std::wstring& correct, bool to_lower = false)
{
    init_locale();
    SymbolNormalizer normalize("../misc/icu/normalization/", "resembla", "nfkc", to_lower);

    auto answer = normalize(input);
#ifdef DEBUG
    std::wcerr << "input  : \"" << input << L"\"" << std::endl;
    std::wcerr << "answer : \"" << answer << L"\"" << std::endl;
    std::wcerr << "correct: \"" << correct << L"\"" << std::endl;

    std::cerr << "dump input  : ";
    for(auto c: input){
        std::cerr << std::hex << (int)c << " ";
    }
    std::cerr << std::endl;
    std::cerr << "dump answer : ";
    for(auto c: answer){
        std::cerr << std::hex << (int)c << " ";
    }
    std::cerr << std::endl;
    std::cerr << "dump correct: ";
    for(auto c: correct){
        std::cerr << std::hex << (int)c << " ";
    }
    std::cerr << std::endl;
#endif
    CHECK(answer == correct);
}

TEST_CASE( "empty symbol normalizer", "[language]" ) {
    std::string input = "ＨｅLLo、＠＄％!！？！!";

    SymbolNormalizer normalize_nothing("", "", "", false);
    CHECK(normalize_nothing(input) == "ＨｅLLo、＠＄％!！？！!");

    SymbolNormalizer normalize_case("", "", "", true);
    CHECK(normalize_case(input) == "ｈｅllo、＠＄％!！？！!");
}

TEST_CASE( "normalize symbols", "[language]" ) {
    test_symbol_normalizer_noramlize_symbol(L"", L"");

    test_symbol_normalizer_noramlize_symbol(L"テスト", L"テスト");
    test_symbol_normalizer_noramlize_symbol(L"ｱｶｻﾀﾅ", L"アカサタナ");

    test_symbol_normalizer_noramlize_symbol(L"TEST", L"TEST");
    test_symbol_normalizer_noramlize_symbol(L"test", L"test");
    test_symbol_normalizer_noramlize_symbol(L"ＡＢＣＤＥ", L"ABCDE");

    test_symbol_normalizer_noramlize_symbol(L"ｱﾎﾞｶﾄﾞ", L"アボカド");
    test_symbol_normalizer_noramlize_symbol(L"パﾍﾟット", L"パペット");

    // double quote
    test_symbol_normalizer_noramlize_symbol(L"\u0022\u201c\u201d", L"\u0022\u0022\u0022");
    // single quote
    test_symbol_normalizer_noramlize_symbol(L"\u0027\u2018\u2019", L"\u0027\u0027\u0027");
    // space
    test_symbol_normalizer_noramlize_symbol(L"\u0020\u200A\uFEFF\u000D\u000A", L"\u0020\u0020\u0020\u0020\u0020");
    // hyphen
    test_symbol_normalizer_noramlize_symbol(L"\u002D\u00AD\u02D7\u058A\u2010\u2011\u2012\u2013\u2043\u207B\u208B\u2212", L"\u002D\u002D\u002D\u002D\u002D\u002D\u002D\u002D\u002D\u002D\u002D\u002D");
    // macron
    test_symbol_normalizer_noramlize_symbol(L"\u30FC\u2014\u2015\u2500\u2501\uFE63\uFF0D\uFF70", L"\u30FC\u30FC\u30FC\u30FC\u30FC\u30FC\u30FC\u30FC");
    // tilde
    test_symbol_normalizer_noramlize_symbol(L"\u007E\u301C\uFF5E\u02DC\u1FC0\u2053\u223C\u223F\u3030", L"\u007E\u007E\u007E\u007E\u007E\u007E\u007E\u007E\u007E");
    // yen sign
    test_symbol_normalizer_noramlize_symbol(L"\u00A5\u005C\uFFE5", L"\u00A5\u00A5\u00A5");

    test_symbol_normalizer_noramlize_symbol(L"0123456789", L"0123456789");
    test_symbol_normalizer_noramlize_symbol(L"０１２３４５６７８９", L"0123456789");

    test_symbol_normalizer_noramlize_symbol(L"ABCDEFGHIJKLMNOPQRSTUVWXYZ", L"ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    test_symbol_normalizer_noramlize_symbol(L"ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺ", L"ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    test_symbol_normalizer_noramlize_symbol(L"abcdefghijklmnopqrstuvwxyz", L"abcdefghijklmnopqrstuvwxyz");
    test_symbol_normalizer_noramlize_symbol(L"ａｂｃｄｅｆｇｈｉｊｋｌｍｎｏｐｑｒｓｔｕｖｗｘｙｚ", L"abcdefghijklmnopqrstuvwxyz");

    test_symbol_normalizer_noramlize_symbol(L"!\"#$%&'()*+,-./:;<>?@[\\]^_`{|}", L"!\"#$%&'()*+,-./:;<>?@[¥]^_`{|}");
    test_symbol_normalizer_noramlize_symbol(L"！“”＃＄％＆‘’（）＊＋，−．／：；＜＞？＠［¥￥］＾＿｀｛｜｝", L"!\"\"#$%&''()*+,-./:;<>?@[¥¥]^_`{|}");
    test_symbol_normalizer_noramlize_symbol(L"=。、・「」", L"=。、・「」");
    test_symbol_normalizer_noramlize_symbol(L"＝｡､･｢｣", L"=。、・「」");

    test_symbol_normalizer_noramlize_symbol(L"テスト!", L"テスト!");
    test_symbol_normalizer_noramlize_symbol(L"テスト！", L"テスト!");
    test_symbol_normalizer_noramlize_symbol(L"テスト?", L"テスト?");
    test_symbol_normalizer_noramlize_symbol(L"テスト？", L"テスト?");
    test_symbol_normalizer_noramlize_symbol(L"テストです．", L"テストです.");

    test_symbol_normalizer_noramlize_symbol(L"こんにちは。テストです", L"こんにちは。テストです");
    test_symbol_normalizer_noramlize_symbol(L"Hello, this　is a　test.", L"Hello, this is a test.");

    test_symbol_normalizer_noramlize_symbol(L"Apple, APPLE, apple and ＡＰＰＬＥ", L"apple, apple, apple and apple", true);
}
