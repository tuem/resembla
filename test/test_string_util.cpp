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

using namespace resembla;

template<typename src_type, typename dest_type>
void test_cast_string(const src_type& input, const dest_type& correct)
{
    init_locale();
    dest_type answer = cast_string<dest_type>(input);
    CHECK(answer == correct);
}

void test_cast_string_string_string(const std::string& input, const std::string& correct)
{
    test_cast_string(input, correct);
}

void test_cast_string_wstring_wstring(const std::wstring& input, const std::wstring& correct)
{
    test_cast_string(input, correct);
}

void test_cast_string_string_wstring(const std::string& input, const std::wstring& correct)
{
    test_cast_string(input, correct);
}

void test_cast_string_wstring_string(const std::wstring& input, const std::string& correct)
{
    test_cast_string(input, correct);
}

TEST_CASE( "string to string", "[language]" ) {
    test_cast_string_string_string("", "");
    test_cast_string_string_string("ABC123", "ABC123");
    test_cast_string_string_string("ｱﾎﾞｶﾄﾞ", "ｱﾎﾞｶﾄﾞ");
    test_cast_string_string_string("テスト", "テスト");
    test_cast_string_string_string("漢字", "漢字");
    test_cast_string_string_string("このﾃｽﾄはcast_stringを実行します。", "このﾃｽﾄはcast_stringを実行します。");
}

TEST_CASE( "wstring to wstring", "[language]" ) {
    test_cast_string_wstring_wstring(L"", L"");
    test_cast_string_wstring_wstring(L"ABC123", L"ABC123");
    test_cast_string_wstring_wstring(L"ｱﾎﾞｶﾄﾞ", L"ｱﾎﾞｶﾄﾞ");
    test_cast_string_wstring_wstring(L"テスト", L"テスト");
    test_cast_string_wstring_wstring(L"漢字", L"漢字");
    test_cast_string_wstring_wstring(L"このﾃｽﾄはcast_stringを実行します。", L"このﾃｽﾄはcast_stringを実行します。");
}

TEST_CASE( "string to wstring", "[language]" ) {
    test_cast_string_string_wstring("", L"");
    test_cast_string_string_wstring("ABC123", L"ABC123");
    test_cast_string_string_wstring("ｱﾎﾞｶﾄﾞ", L"ｱﾎﾞｶﾄﾞ");
    test_cast_string_string_wstring("テスト", L"テスト");
    test_cast_string_string_wstring("漢字", L"漢字");
    test_cast_string_string_wstring("このﾃｽﾄはcast_stringを実行します。", L"このﾃｽﾄはcast_stringを実行します。");
}

TEST_CASE( "wstring to string", "[language]" ) {
    test_cast_string_wstring_string(L"", "");
    test_cast_string_wstring_string(L"ABC123", "ABC123");
    test_cast_string_wstring_string(L"ｱﾎﾞｶﾄﾞ", "ｱﾎﾞｶﾄﾞ");
    test_cast_string_wstring_string(L"テスト", "テスト");
    test_cast_string_wstring_string(L"漢字", "漢字");
    test_cast_string_wstring_string(L"このﾃｽﾄはcast_stringを実行します。", "このﾃｽﾄはcast_stringを実行します。");
}

TEST_CASE( "split strings", "[language]" ) {
    std::string line = "abc\tpqr\txyz";
    CHECK(split(line) == std::vector<std::string>({"abc", "pqr", "xyz"}));
    line = "abc,pqr,xyz";
    CHECK(split(line) == std::vector<std::string>({"abc,pqr,xyz"}));
    CHECK(split(line, ',') == std::vector<std::string>({"abc", "pqr", "xyz"}));
    CHECK(split(line, ',', 2) == std::vector<std::string>({"abc", "pqr,xyz"}));
    line = "abc,xyz,";
    CHECK(split(line, ',') == std::vector<std::string>({"abc", "xyz", ""}));
}
