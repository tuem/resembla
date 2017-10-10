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

#include "measure/romaji_preprocessor.hpp"

using namespace resembla;

const std::string ROMAJI_PREPROCESSOR_MECAB_OPTIONS = "-d ../misc/mecab_dic/unidic/unidic-mecab-2.1.2_bin -Odump";
const size_t ROMAJI_PREPROCESSOR_MECAB_FUTURE_POS = 9;
const std::string ROMAJI_PREPROCESSOR_MECAB_ROMAJI_OF_MARKS = "記号";

void test_romaji_preprocessor(const std::string& input, const std::string& correct)
{
    init_locale();
    RomajiPreprocessor preprocess(
        ROMAJI_PREPROCESSOR_MECAB_OPTIONS,
        ROMAJI_PREPROCESSOR_MECAB_FUTURE_POS,
        ROMAJI_PREPROCESSOR_MECAB_ROMAJI_OF_MARKS,
        true);
    std::wstring winput = cast_string<std::wstring>(input);
    std::string answer = cast_string<std::string>(preprocess(winput));
#ifdef DEBUG
    std::cerr << "input text: " << input <<  std::endl;
    std::cerr << "parsed text: " << answer <<  std::endl;
#endif
    CHECK(answer == correct);
}

TEST_CASE( "romaji: empty", "[language]" ) {
    std::string input = "";
    std::string correct = "";
    test_romaji_preprocessor(input, correct);
}

TEST_CASE( "romaji: KAIRO", "[language]" ) {
    std::string input = "カイロ";
    std::string correct = "KAIRO";
    test_romaji_preprocessor(input, correct);
}

TEST_CASE( "romaji: KAtTO", "[language]" ) {
    std::string input = "カット";
    std::string correct = "KAtTO";
    test_romaji_preprocessor(input, correct);
}

TEST_CASE( "romaji: GwaTU", "[language]" ) {
    std::string input = "グヮツ";
    std::string correct = "GwaTU";
    test_romaji_preprocessor(input, correct);
}

TEST_CASE( "romaji: SyaTU", "[language]" ) {
    std::string input = "シャツ";
    std::string correct = "SyaTU";
    test_romaji_preprocessor(input, correct);
}

TEST_CASE( "romaji: VAIVURE-Syon", "[language]" ) {
    std::string input = "ヴァイヴレーション";
    std::string correct = "VAIVURE-Syon";
    test_romaji_preprocessor(input, correct);
}

TEST_CASE( "romaji: GyapPU", "[language]" ) {
    std::string input = "ギャップ";
    std::string correct = "GyapPU";
    test_romaji_preprocessor(input, correct);
}

TEST_CASE( "romaji: SUKAsSyu", "[language]" ) {
    std::string input = "スカッシュ";
    std::string correct = "SUKAsSyu";
    test_romaji_preprocessor(input, correct);
}

TEST_CASE( "romaji: HuaITeinGU", "[language]" ) {
    std::string input = "ファイティング";
    std::string correct = "HuaITeinGU";
    test_romaji_preprocessor(input, correct);
}

TEST_CASE( "romaji: EAui-VU", "[language]" ) {
    std::string input = "エアウィーヴ";
    std::string correct = "EAui-VU";
    test_romaji_preprocessor(input, correct);
}

TEST_CASE( "romaji: all letters", "[language]" ) {
    test_romaji_preprocessor("アイウエオ", "AIUEO");
    test_romaji_preprocessor("ァィゥェォ", "aiueo");
    test_romaji_preprocessor("カキクケコ", "KAKIKUKEKO");
    test_romaji_preprocessor("ガギグゲゴ", "GAGIGUGEGO");
    test_romaji_preprocessor("サシスセソ", "SASISUSESO");
    test_romaji_preprocessor("ザジズゼゾ", "ZAZIZUZEZO");
    test_romaji_preprocessor("タチツテト", "TATITUTETO");
    test_romaji_preprocessor("ダヂヅデド", "DADIDUDEDO");
    test_romaji_preprocessor("ナニヌネノ", "NANINUNENO");
    test_romaji_preprocessor("ハヒフヘホ", "HAHIHUHEHO");
    test_romaji_preprocessor("バビブベボ", "BABIBUBEBO");
    test_romaji_preprocessor("パピプペポ", "PAPIPUPEPO");
    test_romaji_preprocessor("マミムメモ", "MAMIMUMEMO");
    test_romaji_preprocessor("ヤユヨ", "YAYUYO");
    test_romaji_preprocessor("ラリルレロ", "RARIRURERO");
    test_romaji_preprocessor("ヴァヴィヴヴェヴォ", "VAVIVUVEVO");
    test_romaji_preprocessor("ワヲン", "WAWOn");
    test_romaji_preprocessor("ヮヵヶー", "wakake-");
    test_romaji_preprocessor("ッ", "tu");
    test_romaji_preprocessor("ャ", "ya");
    test_romaji_preprocessor("ュ", "yu");
    test_romaji_preprocessor("ョ", "yo");
    test_romaji_preprocessor("キャキュキョ", "KyaKyuKyo");
    test_romaji_preprocessor("ギャギュギョ", "GyaGyuGyo");
    test_romaji_preprocessor("シャシュショ", "SyaSyuSyo");
    test_romaji_preprocessor("ジャジュジョ", "ZyaZyuZyo");
    test_romaji_preprocessor("チャチュチョ", "TyaTyuTyo");
    test_romaji_preprocessor("ヂャヂュヂョ", "DyaDyuDyo");
    test_romaji_preprocessor("ニャニュニョ", "NyaNyuNyo");
    test_romaji_preprocessor("ヒャヒュヒョ", "HyaHyuHyo");
    test_romaji_preprocessor("ビャビュビョ", "ByaByuByo");
    test_romaji_preprocessor("ピャピュピョ", "PyaPyuPyo");
    test_romaji_preprocessor("ミャミュミョ", "MyaMyuMyo");
    test_romaji_preprocessor("リャリュリョ", "RyaRyuRyo");
    test_romaji_preprocessor("ウィウェウォ", "uiueuo");
    test_romaji_preprocessor("チェティ", "TieTei");
    test_romaji_preprocessor("ファフィフェフォ", "HuaHuiHueHuo");
    test_romaji_preprocessor("ッカッキックッケッコ", "kKAkKIkKUkKEkKO");
    test_romaji_preprocessor("ッガッギッグッゲッゴ", "gGAgGIgGUgGEgGO");
    test_romaji_preprocessor("ッサッシッスッセッソ", "sSAsSIsSUsSEsSO");
    test_romaji_preprocessor("ッザッジッズッゼッゾ", "zZAzZIzZUzZEzZO");
    test_romaji_preprocessor("ッタッチッツッテット", "tTAtTItTUtTEtTO");
    test_romaji_preprocessor("ッダッヂッヅッデッド", "dDAdDIdDUdDEdDO");
    test_romaji_preprocessor("ッナッニッヌッネッノ", "nNAnNInNUnNEnNO");
    test_romaji_preprocessor("ッハッヒッフッヘッホ", "hHAhHIhHUhHEhHO");
    test_romaji_preprocessor("ッバッビッブッベッボ", "bBAbBIbBUbBEbBO");
    test_romaji_preprocessor("ッパッピップッペッポ", "pPApPIpPUpPEpPO");
    test_romaji_preprocessor("ッマッミッムッメッモ", "mMAmMImMUmMEmMO");
    test_romaji_preprocessor("ッヤッユッヨ", "yYAyYUyYO");
    test_romaji_preprocessor("ッラッリッルッレッロ", "rRArRIrRUrRErRO");
    test_romaji_preprocessor("ッワッヲッヴ", "wWAwWOvVU");
}
