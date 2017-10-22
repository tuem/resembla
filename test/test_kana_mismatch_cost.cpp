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

#include "measure/kana_mismatch_cost.hpp"

using namespace resembla;

void test_kana_mismatch_cost(const wchar_t a, const wchar_t b,
        std::string mismatch_cost_file_path, double correct)
{
    init_locale();
    KanaMismatchCost<string_type> cost(mismatch_cost_file_path);
#ifdef DEBUG
    std::wcerr << L"kana_mismatch_cost: " << L"cost('" << a << L"','" << b << L"')=" << cost(a, b) << std::endl;
#endif
    CHECK(cost(a, b) == Approx(correct));
}

TEST_CASE( "check kana_mismatch_cost when input letters are the same", "[language]" ) {
    test_kana_mismatch_cost(L'ア', L'ア', "../example/conf/kana_mismatch_cost.tsv", 0.0);
    test_kana_mismatch_cost(L'キ', L'キ', "../example/conf/kana_mismatch_cost.tsv", 0.0);
    test_kana_mismatch_cost(L'ブ', L'ブ', "../example/conf/kana_mismatch_cost.tsv", 0.0);
    test_kana_mismatch_cost(L'ン', L'ン', "../example/conf/kana_mismatch_cost.tsv", 0.0);
    test_kana_mismatch_cost(L'あ', L'あ', "../example/conf/kana_mismatch_cost.tsv", 0.0);
    test_kana_mismatch_cost(L'宛', L'宛', "../example/conf/kana_mismatch_cost.tsv", 0.0);
}

TEST_CASE( "check kana_mismatch_cost with typical weight settings", "[language]" ) {
    test_kana_mismatch_cost(L'ア', L'ァ', "../example/conf/kana_mismatch_cost.tsv", 0.2);
    test_kana_mismatch_cost(L'イ', L'ィ', "../example/conf/kana_mismatch_cost.tsv", 0.2);
    test_kana_mismatch_cost(L'ウ', L'ゥ', "../example/conf/kana_mismatch_cost.tsv", 0.2);
    test_kana_mismatch_cost(L'エ', L'ェ', "../example/conf/kana_mismatch_cost.tsv", 0.2);
    test_kana_mismatch_cost(L'オ', L'ォ', "../example/conf/kana_mismatch_cost.tsv", 0.2);
    test_kana_mismatch_cost(L'ウ', L'ヴ', "../example/conf/kana_mismatch_cost.tsv", 0.3);
    test_kana_mismatch_cost(L'カ', L'ガ', "../example/conf/kana_mismatch_cost.tsv", 0.3);
    test_kana_mismatch_cost(L'キ', L'ギ', "../example/conf/kana_mismatch_cost.tsv", 0.3);
    test_kana_mismatch_cost(L'ク', L'グ', "../example/conf/kana_mismatch_cost.tsv", 0.3);
    test_kana_mismatch_cost(L'ケ', L'ゲ', "../example/conf/kana_mismatch_cost.tsv", 0.3);
    test_kana_mismatch_cost(L'コ', L'ゴ', "../example/conf/kana_mismatch_cost.tsv", 0.3);
    test_kana_mismatch_cost(L'サ', L'ザ', "../example/conf/kana_mismatch_cost.tsv", 0.3);
    test_kana_mismatch_cost(L'シ', L'ジ', "../example/conf/kana_mismatch_cost.tsv", 0.3);
    test_kana_mismatch_cost(L'ジ', L'ヂ', "../example/conf/kana_mismatch_cost.tsv", 0.1);
    test_kana_mismatch_cost(L'ス', L'ズ', "../example/conf/kana_mismatch_cost.tsv", 0.3);
    test_kana_mismatch_cost(L'ズ', L'ヅ', "../example/conf/kana_mismatch_cost.tsv", 0.1);
    test_kana_mismatch_cost(L'セ', L'ゼ', "../example/conf/kana_mismatch_cost.tsv", 0.3);
    test_kana_mismatch_cost(L'ソ', L'ゾ', "../example/conf/kana_mismatch_cost.tsv", 0.3);
    test_kana_mismatch_cost(L'タ', L'ダ', "../example/conf/kana_mismatch_cost.tsv", 0.3);
    test_kana_mismatch_cost(L'チ', L'ヂ', "../example/conf/kana_mismatch_cost.tsv", 0.3);
    test_kana_mismatch_cost(L'ツ', L'ヅ', "../example/conf/kana_mismatch_cost.tsv", 0.3);
    test_kana_mismatch_cost(L'ツ', L'ッ', "../example/conf/kana_mismatch_cost.tsv", 0.2);
    test_kana_mismatch_cost(L'テ', L'デ', "../example/conf/kana_mismatch_cost.tsv", 0.3);
    test_kana_mismatch_cost(L'ト', L'ド', "../example/conf/kana_mismatch_cost.tsv", 0.3);
    test_kana_mismatch_cost(L'ハ', L'バ', "../example/conf/kana_mismatch_cost.tsv", 0.3);
    test_kana_mismatch_cost(L'ハ', L'パ', "../example/conf/kana_mismatch_cost.tsv", 0.3);
    test_kana_mismatch_cost(L'バ', L'パ', "../example/conf/kana_mismatch_cost.tsv", 0.3);
    test_kana_mismatch_cost(L'ヒ', L'ビ', "../example/conf/kana_mismatch_cost.tsv", 0.3);
    test_kana_mismatch_cost(L'ヒ', L'ピ', "../example/conf/kana_mismatch_cost.tsv", 0.3);
    test_kana_mismatch_cost(L'ビ', L'ピ', "../example/conf/kana_mismatch_cost.tsv", 0.3);
    test_kana_mismatch_cost(L'フ', L'ブ', "../example/conf/kana_mismatch_cost.tsv", 0.3);
    test_kana_mismatch_cost(L'フ', L'プ', "../example/conf/kana_mismatch_cost.tsv", 0.3);
    test_kana_mismatch_cost(L'ブ', L'プ', "../example/conf/kana_mismatch_cost.tsv", 0.3);
    test_kana_mismatch_cost(L'ヘ', L'ベ', "../example/conf/kana_mismatch_cost.tsv", 0.3);
    test_kana_mismatch_cost(L'ヘ', L'ペ', "../example/conf/kana_mismatch_cost.tsv", 0.3);
    test_kana_mismatch_cost(L'ベ', L'ペ', "../example/conf/kana_mismatch_cost.tsv", 0.3);
    test_kana_mismatch_cost(L'ホ', L'ボ', "../example/conf/kana_mismatch_cost.tsv", 0.3);
    test_kana_mismatch_cost(L'ホ', L'ポ', "../example/conf/kana_mismatch_cost.tsv", 0.3);
    test_kana_mismatch_cost(L'ボ', L'ポ', "../example/conf/kana_mismatch_cost.tsv", 0.3);
    test_kana_mismatch_cost(L'ヤ', L'ャ', "../example/conf/kana_mismatch_cost.tsv", 0.3);
    test_kana_mismatch_cost(L'ユ', L'ュ', "../example/conf/kana_mismatch_cost.tsv", 0.3);
    test_kana_mismatch_cost(L'ヨ', L'ョ', "../example/conf/kana_mismatch_cost.tsv", 0.3);
    test_kana_mismatch_cost(L'ヤ', L'ア', "../example/conf/kana_mismatch_cost.tsv", 0.4);
    test_kana_mismatch_cost(L'ユ', L'ウ', "../example/conf/kana_mismatch_cost.tsv", 0.4);
    test_kana_mismatch_cost(L'ヨ', L'オ', "../example/conf/kana_mismatch_cost.tsv", 0.4);
    test_kana_mismatch_cost(L'ヤ', L'ァ', "../example/conf/kana_mismatch_cost.tsv", 0.4);
    test_kana_mismatch_cost(L'ユ', L'ゥ', "../example/conf/kana_mismatch_cost.tsv", 0.4);
    test_kana_mismatch_cost(L'ヨ', L'ォ', "../example/conf/kana_mismatch_cost.tsv", 0.4);
    test_kana_mismatch_cost(L'ャ', L'ァ', "../example/conf/kana_mismatch_cost.tsv", 0.4);
    test_kana_mismatch_cost(L'ュ', L'ゥ', "../example/conf/kana_mismatch_cost.tsv", 0.4);
    test_kana_mismatch_cost(L'ョ', L'ォ', "../example/conf/kana_mismatch_cost.tsv", 0.4);
    test_kana_mismatch_cost(L'ア', L'ワ', "../example/conf/kana_mismatch_cost.tsv", 0.4);
    test_kana_mismatch_cost(L'オ', L'ヲ', "../example/conf/kana_mismatch_cost.tsv", 0.1);
    test_kana_mismatch_cost(L'ン', L'ヌ', "../example/conf/kana_mismatch_cost.tsv", 0.4);
    test_kana_mismatch_cost(L'ン', L'ウ', "../example/conf/kana_mismatch_cost.tsv", 0.4);
    test_kana_mismatch_cost(L'ア', L'ー', "../example/conf/kana_mismatch_cost.tsv", 0.4);
    test_kana_mismatch_cost(L'イ', L'ー', "../example/conf/kana_mismatch_cost.tsv", 0.4);
    test_kana_mismatch_cost(L'ウ', L'ー', "../example/conf/kana_mismatch_cost.tsv", 0.4);
    test_kana_mismatch_cost(L'エ', L'ー', "../example/conf/kana_mismatch_cost.tsv", 0.4);
    test_kana_mismatch_cost(L'オ', L'ー', "../example/conf/kana_mismatch_cost.tsv", 0.4);
    test_kana_mismatch_cost(L'ァ', L'ー', "../example/conf/kana_mismatch_cost.tsv", 0.4);
    test_kana_mismatch_cost(L'ィ', L'ー', "../example/conf/kana_mismatch_cost.tsv", 0.4);
    test_kana_mismatch_cost(L'ゥ', L'ー', "../example/conf/kana_mismatch_cost.tsv", 0.4);
    test_kana_mismatch_cost(L'ェ', L'ー', "../example/conf/kana_mismatch_cost.tsv", 0.4);
    test_kana_mismatch_cost(L'ォ', L'ー', "../example/conf/kana_mismatch_cost.tsv", 0.4);
    test_kana_mismatch_cost(L'ワ', L'ヮ', "../example/conf/kana_mismatch_cost.tsv", 0.2);
    test_kana_mismatch_cost(L'カ', L'ヵ', "../example/conf/kana_mismatch_cost.tsv", 0.2);
    test_kana_mismatch_cost(L'ケ', L'ヶ', "../example/conf/kana_mismatch_cost.tsv", 0.2);
    test_kana_mismatch_cost(L'ア', L'イ', "../example/conf/kana_mismatch_cost.tsv", 0.9);
    test_kana_mismatch_cost(L'ア', L'カ', "../example/conf/kana_mismatch_cost.tsv", 0.6);
    test_kana_mismatch_cost(L'サ', L'カ', "../example/conf/kana_mismatch_cost.tsv", 0.8);
    test_kana_mismatch_cost(L'チ', L'ニ', "../example/conf/kana_mismatch_cost.tsv", 0.8);
}

TEST_CASE( "check kana_mismatch_cost when input letter is not a kana", "[language]" ) {
    test_kana_mismatch_cost(L'ア', L'あ', "../example/conf/kana_mismatch_cost.tsv", 1.0);
    test_kana_mismatch_cost(L'ア', L'宛', "../example/conf/kana_mismatch_cost.tsv", 1.0);
    test_kana_mismatch_cost(L'あ', L'宛', "../example/conf/kana_mismatch_cost.tsv", 1.0);
}
