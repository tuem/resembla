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

#include "measure/romaji_sequence_builder.hpp"

using namespace resembla;

const std::string ROMAJI_SEQUENCE_PARSER_MECAB_OPTIONS = "-d ../misc/mecab_dic/unidic/unidic-mecab-2.1.2_bin -Odump";
const size_t ROMAJI_SEQUENCE_PARSER_MECAB_FUTURE_POS = 9;
const std::string ROMAJI_SEQUENCE_PARSER_MECAB_ROMAJI_OF_MARKS = "記号";

void test_romaji_sequence_builder_build_indexing_text(const std::string& input, const std::string& correct)
{
    init_locale();
    RomajiSequenceBuilder preprocess(ROMAJI_SEQUENCE_PARSER_MECAB_OPTIONS, ROMAJI_SEQUENCE_PARSER_MECAB_FUTURE_POS, ROMAJI_SEQUENCE_PARSER_MECAB_ROMAJI_OF_MARKS, true);
    std::wstring winput = cast_string<std::wstring>(input);
    std::string answer = cast_string<std::string>(preprocess.index(winput));
#ifdef DEBUG
    std::cerr << "input text: " << input <<  std::endl;
    std::cerr << "parsed text: " << answer <<  std::endl;
#endif
    CHECK(answer == correct);
}

TEST_CASE( "romaji: empty", "[language]" ) {
    std::string input = "";
    std::string correct = "";
    test_romaji_sequence_builder_build_indexing_text(input, correct);
}

TEST_CASE( "romaji: KAIRO", "[language]" ) {
    std::string input = "カイロ";
    std::string correct = "KAIRO";
    test_romaji_sequence_builder_build_indexing_text(input, correct);
}

TEST_CASE( "romaji: KAtTO", "[language]" ) {
    std::string input = "カット";
    std::string correct = "KAtTO";
    test_romaji_sequence_builder_build_indexing_text(input, correct);
}

TEST_CASE( "romaji: GwaTU", "[language]" ) {
    std::string input = "グヮツ";
    std::string correct = "GwaTU";
    test_romaji_sequence_builder_build_indexing_text(input, correct);
}

TEST_CASE( "romaji: SyaTU", "[language]" ) {
    std::string input = "シャツ";
    std::string correct = "SyaTU";
    test_romaji_sequence_builder_build_indexing_text(input, correct);
}

TEST_CASE( "romaji: VAIVURE-Syon", "[language]" ) {
    std::string input = "ヴァイヴレーション";
    std::string correct = "VAIVURE-Syon";
    test_romaji_sequence_builder_build_indexing_text(input, correct);
}

TEST_CASE( "romaji: GyapPU", "[language]" ) {
    std::string input = "ギャップ";
    std::string correct = "GyapPU";
    test_romaji_sequence_builder_build_indexing_text(input, correct);
}

TEST_CASE( "romaji: SUKAsSyu", "[language]" ) {
    std::string input = "スカッシュ";
    std::string correct = "SUKAsSyu";
    test_romaji_sequence_builder_build_indexing_text(input, correct);
}

TEST_CASE( "romaji: HuaITeinGU", "[language]" ) {
    std::string input = "ファイティング";
    std::string correct = "HuaITeinGU";
    test_romaji_sequence_builder_build_indexing_text(input, correct);
}

TEST_CASE( "romaji: EAui-VU", "[language]" ) {
    std::string input = "エアウィーヴ";
    std::string correct = "EAui-VU";
    test_romaji_sequence_builder_build_indexing_text(input, correct);
}

TEST_CASE( "romaji: all letters", "[language]" ) {
    test_romaji_sequence_builder_build_indexing_text("アイウエオ", "AIUEO");
    test_romaji_sequence_builder_build_indexing_text("ァィゥェォ", "aiueo");
    test_romaji_sequence_builder_build_indexing_text("カキクケコ", "KAKIKUKEKO");
    test_romaji_sequence_builder_build_indexing_text("ガギグゲゴ", "GAGIGUGEGO");
    test_romaji_sequence_builder_build_indexing_text("サシスセソ", "SASISUSESO");
    test_romaji_sequence_builder_build_indexing_text("ザジズゼゾ", "ZAZIZUZEZO");
    test_romaji_sequence_builder_build_indexing_text("タチツテト", "TATITUTETO");
    test_romaji_sequence_builder_build_indexing_text("ダヂヅデド", "DADIDUDEDO");
    test_romaji_sequence_builder_build_indexing_text("ナニヌネノ", "NANINUNENO");
    test_romaji_sequence_builder_build_indexing_text("ハヒフヘホ", "HAHIHUHEHO");
    test_romaji_sequence_builder_build_indexing_text("バビブベボ", "BABIBUBEBO");
    test_romaji_sequence_builder_build_indexing_text("パピプペポ", "PAPIPUPEPO");
    test_romaji_sequence_builder_build_indexing_text("マミムメモ", "MAMIMUMEMO");
    test_romaji_sequence_builder_build_indexing_text("ヤユヨ", "YAYUYO");
    test_romaji_sequence_builder_build_indexing_text("ラリルレロ", "RARIRURERO");
    test_romaji_sequence_builder_build_indexing_text("ヴァヴィヴヴェヴォ", "VAVIVUVEVO");
    test_romaji_sequence_builder_build_indexing_text("ワヲン", "WAWOn");
    test_romaji_sequence_builder_build_indexing_text("ヮヵヶー", "wakake-");
    test_romaji_sequence_builder_build_indexing_text("ッ", "tu");
    test_romaji_sequence_builder_build_indexing_text("ャ", "ya");
    test_romaji_sequence_builder_build_indexing_text("ュ", "yu");
    test_romaji_sequence_builder_build_indexing_text("ョ", "yo");
    test_romaji_sequence_builder_build_indexing_text("キャキュキョ", "KyaKyuKyo");
    test_romaji_sequence_builder_build_indexing_text("ギャギュギョ", "GyaGyuGyo");
    test_romaji_sequence_builder_build_indexing_text("シャシュショ", "SyaSyuSyo");
    test_romaji_sequence_builder_build_indexing_text("ジャジュジョ", "ZyaZyuZyo");
    test_romaji_sequence_builder_build_indexing_text("チャチュチョ", "TyaTyuTyo");
    test_romaji_sequence_builder_build_indexing_text("ヂャヂュヂョ", "DyaDyuDyo");
    test_romaji_sequence_builder_build_indexing_text("ニャニュニョ", "NyaNyuNyo");
    test_romaji_sequence_builder_build_indexing_text("ヒャヒュヒョ", "HyaHyuHyo");
    test_romaji_sequence_builder_build_indexing_text("ビャビュビョ", "ByaByuByo");
    test_romaji_sequence_builder_build_indexing_text("ピャピュピョ", "PyaPyuPyo");
    test_romaji_sequence_builder_build_indexing_text("ミャミュミョ", "MyaMyuMyo");
    test_romaji_sequence_builder_build_indexing_text("リャリュリョ", "RyaRyuRyo");
    test_romaji_sequence_builder_build_indexing_text("ウィウェウォ", "uiueuo");
    test_romaji_sequence_builder_build_indexing_text("チェティ", "TieTei");
    test_romaji_sequence_builder_build_indexing_text("ファフィフェフォ", "HuaHuiHueHuo");
    test_romaji_sequence_builder_build_indexing_text("ッカッキックッケッコ", "kKAkKIkKUkKEkKO");
    test_romaji_sequence_builder_build_indexing_text("ッガッギッグッゲッゴ", "gGAgGIgGUgGEgGO");
    test_romaji_sequence_builder_build_indexing_text("ッサッシッスッセッソ", "sSAsSIsSUsSEsSO");
    test_romaji_sequence_builder_build_indexing_text("ッザッジッズッゼッゾ", "zZAzZIzZUzZEzZO");
    test_romaji_sequence_builder_build_indexing_text("ッタッチッツッテット", "tTAtTItTUtTEtTO");
    test_romaji_sequence_builder_build_indexing_text("ッダッヂッヅッデッド", "dDAdDIdDUdDEdDO");
    test_romaji_sequence_builder_build_indexing_text("ッナッニッヌッネッノ", "nNAnNInNUnNEnNO");
    test_romaji_sequence_builder_build_indexing_text("ッハッヒッフッヘッホ", "hHAhHIhHUhHEhHO");
    test_romaji_sequence_builder_build_indexing_text("ッバッビッブッベッボ", "bBAbBIbBUbBEbBO");
    test_romaji_sequence_builder_build_indexing_text("ッパッピップッペッポ", "pPApPIpPUpPEpPO");
    test_romaji_sequence_builder_build_indexing_text("ッマッミッムッメッモ", "mMAmMImMUmMEmMO");
    test_romaji_sequence_builder_build_indexing_text("ッヤッユッヨ", "yYAyYUyYO");
    test_romaji_sequence_builder_build_indexing_text("ッラッリッルッレッロ", "rRArRIrRUrRErRO");
    test_romaji_sequence_builder_build_indexing_text("ッワッヲッヴ", "wWAwWOvVU");
}
