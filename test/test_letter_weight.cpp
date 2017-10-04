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

#include "Catch/catch.hpp"

#include "string_util.hpp"

#include "measure/letter_weight.hpp"

using namespace resembla;

TEST_CASE( "check letter_weight with kana letters", "[language]" ) {
    init_locale();

    double base_weight = 1.0;
    double delete_insert_ratio = 0.8;
    std::string letter_weight_file_path = "../example/conf/kana_weight.tsv";
    LetterWeight<string_type> kana_weight(base_weight, delete_insert_ratio, letter_weight_file_path);

    CHECK(kana_weight(L'ア', true) == Approx(0.56));
    CHECK(kana_weight(L'イ', false) == Approx(0.7));
    CHECK(kana_weight(L'ゥ', true) == Approx(0.32));
    CHECK(kana_weight(L'ー') == Approx(0.5));
    CHECK(kana_weight(L'ン') == Approx(0.5));
    CHECK(kana_weight(L'カ') == Approx(1.0));
    CHECK(kana_weight(L'ジ') == Approx(1.0));
    CHECK(kana_weight(L'プ') == Approx(1.0));
    CHECK(kana_weight(L'え') == Approx(1.0));
    CHECK(kana_weight(L'御') == Approx(1.0));
    CHECK(kana_weight(L'？') == Approx(1.0));
    CHECK(kana_weight(L'.') == Approx(1.0));

    base_weight = 0.6;
    delete_insert_ratio = 1.7;
    LetterWeight<string_type> kana_weight2(base_weight, delete_insert_ratio, letter_weight_file_path);
    CHECK(kana_weight2(L'ア', true) == Approx(0.714));
    CHECK(kana_weight2(L'イ', false) == Approx(0.42));
}
