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
#include <codecvt>
#include "measure/word_preprocessor.hpp"
#include "string_util.hpp"

#include "Catch/catch.hpp"

using namespace resembla;

TEST_CASE( "parse a word", "[language]" ) {
    std::ios_base::sync_with_stdio(false);
    std::locale::global(std::locale(std::locale(""), new std::codecvt_utf8<wchar_t>));
    setlocale(LC_ALL, "");
    std::wcin.imbue(std::locale());
    std::wcout.imbue(std::locale());
    std::wcerr.imbue(std::locale());

    std::string mecab_options = "";
    WordPreprocessor<string_type> preprocess(mecab_options);
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;

    std::wstring text = L"テスト";
    auto words = preprocess(text);
    CHECK(words.size() == 1);

    auto& m = words[0];
    CHECK(converter.to_bytes(m.surface) == "テスト");
    CHECK(m.feature.size() == 9);
    std::vector<std::string> features = {"名詞", "サ変接続", "*", "*", "*", "*", "テスト", "テスト", "テスト"};
    for(size_t i = 0; i < features.size(); ++i){
        CHECK(converter.to_bytes(m.feature[i]) == features[i]);
    }
}

TEST_CASE( "parse words", "[language]" ) {
    std::ios_base::sync_with_stdio(false);
    std::locale::global(std::locale(std::locale(""), new std::codecvt_utf8<wchar_t>));
    setlocale(LC_ALL, "");
    std::wcin.imbue(std::locale());
    std::wcout.imbue(std::locale());
    std::wcerr.imbue(std::locale());

    std::string mecab_options = "";
    WordPreprocessor<string_type> preprocess(mecab_options);
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;

    std::wstring text = L"私は考える";
    auto words = preprocess(text);
    CHECK(words.size() == 3);

    auto& m = words[0];
    CHECK(converter.to_bytes(m.surface) == "私");
    CHECK(converter.to_bytes(m.feature[0]) == "名詞");
    m = words[1];
    CHECK(converter.to_bytes(m.surface) == "は");
    CHECK(converter.to_bytes(m.feature[0]) == "助詞");
    m = words[2];
    CHECK(converter.to_bytes(m.surface) == "考える");
    CHECK(converter.to_bytes(m.feature[0]) == "動詞");

    for(auto i = words.begin(); i != words.end(); ++i){
        CHECK(i->feature.size() == 9);
    }
}
