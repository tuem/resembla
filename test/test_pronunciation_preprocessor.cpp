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

#include "measure/pronunciation_preprocessor.hpp"

using namespace resembla;

const std::string PRONUNCIATION_SEQUENCE_PARSER_MECAB_OPTIONS = "-d ../misc/mecab_dic/unidic/unidic-mecab-2.1.2_bin -Odump";
const size_t PRONUNCIATION_SEQUENCE_PARSER_MECAB_FUTURE_POS = 9;
const std::string PRONUNCIATION_SEQUENCE_PARSER_MECAB_PRONUNCIATION_OF_MARKS = "記号";

void test_pronunciation_preprocessor(const std::wstring& input, const std::wstring& correct)
{
    init_locale();
    PronunciationPreprocessor preprocess(
        PRONUNCIATION_SEQUENCE_PARSER_MECAB_OPTIONS,
        PRONUNCIATION_SEQUENCE_PARSER_MECAB_FUTURE_POS,
        PRONUNCIATION_SEQUENCE_PARSER_MECAB_PRONUNCIATION_OF_MARKS);
    auto answer = preprocess(input);
#ifdef DEBUG
    std::wcerr << "input text: " << input <<  std::endl;
    std::wcerr << "pronunciation: " << answer << std::endl;
#endif
    CHECK(answer == correct);
}

TEST_CASE( "parse an empty string to pronunciation sequence", "[language]" ) {
    std::wstring input = L"";
    std::wstring correct = L"";
    test_pronunciation_preprocessor(input, correct);
}

TEST_CASE( "parse a katakana word to pronunciation sequence", "[language]" ) {
    std::wstring input = L"シケン";
    std::wstring correct = L"シケン";
    test_pronunciation_preprocessor(input, correct);
}

TEST_CASE( "parse a hiragana word to pronunciation sequence", "[language]" ) {
    std::wstring input= L"しけん";
    std::wstring correct = L"シケン";
    test_pronunciation_preprocessor(input, correct);
}

TEST_CASE( "parse a kanji word to pronunciation sequence", "[language]" ) {
    std::wstring input = L"試験";
    std::wstring correct = L"シケン";
    test_pronunciation_preprocessor(input, correct);
}

TEST_CASE( "parse a sentence including hiragana, katakana and kanji to pronunciation sequence", "[language]" ) {
    std::wstring input = L"このテストは難関です";
    std::wstring correct = L"コノテストハナンカンデス";
    test_pronunciation_preprocessor(input, correct);
}

TEST_CASE( "parse a sentence including hiragana, katakana, kanji and unknown hiragana word to pronunciation sequence", "[language]" ) {
    std::wstring input = L"平仮名の未知語ぁぃぅぇぉをカタカナに変換する";
    std::wstring correct = L"ヒラガナノミチゴァィゥェォヲカタカナニヘンカンスル";
    test_pronunciation_preprocessor(input, correct);
}

TEST_CASE( "parse a sentence including hiragana, katakana, kanji and unknown katakana word to pronunciation sequence", "[language]" ) {
    std::wstring input = L"カタカナの未知語リセンブラをカタカナに変換する";
    std::wstring correct = L"カタカナノミチゴリセンブラヲカタカナニヘンカンスル";
    test_pronunciation_preprocessor(input, correct);
}

TEST_CASE( "parse a sentence including hiragana, katakana, kanji and unknown kanji word to pronunciation sequence", "[language]" ) {
    std::wstring input = L"漢字の未知語挫宛はそのまま出力される";
    std::wstring correct = L"カンジノミチゴ挫宛ハソノママシュツリョクサレル";
    test_pronunciation_preprocessor(input, correct);
}

TEST_CASE( "parse a sentence including old hiraganas", "[language]" ) {
    std::wstring input = L"ゐゑゔゕゖゝゞゟヰヱヿ";
    std::wstring correct = L"イエヴヵヶヽヾヨリイエコト";
    test_pronunciation_preprocessor(input, correct);
}

TEST_CASE( "parse a sentence including a conjugated form", "[language]" ) {
    std::wstring input = L"今日はもう出かけなきゃ";
    std::wstring correct = L"キョーハモウデカケナキャ";
    test_pronunciation_preprocessor(input, correct);
}

TEST_CASE( "parse a sentence including hiragana, katakana and kanji to text", "[language]" ) {
    std::wstring input = L"このテストは難関です";
    std::wstring correct = L"コノテストハナンカンデス";
    test_pronunciation_preprocessor(input, correct);
}

TEST_CASE( "parse a sentence including hiragana, katakana, kanji and unknown hiragana word to text", "[language]" ) {
    std::wstring input = L"平仮名の未知語ぁぃぅぇぉをカタカナに変換する";
    std::wstring correct = L"ヒラガナノミチゴァィゥェォヲカタカナニヘンカンスル";
    test_pronunciation_preprocessor(input, correct);
}

TEST_CASE( "parse a sentence including hiragana, katakana, kanji and unknown katakana word to text", "[language]" ) {
    std::wstring input = L"カタカナの未知語リセンブラをカタカナに変換する";
    std::wstring correct = L"カタカナノミチゴリセンブラヲカタカナニヘンカンスル";
    test_pronunciation_preprocessor(input, correct);
}

TEST_CASE( "parse a sentence including hiragana, katakana, kanji and unknown kanji word to text", "[language]" ) {
    std::wstring input = L"漢字の未知語挫宛はそのまま出力される";
    std::wstring correct = L"カンジノミチゴ挫宛ハソノママシュツリョクサレル";
    test_pronunciation_preprocessor(input, correct);
}

TEST_CASE( "parse a sentence including old hiraganas to text", "[language]" ) {
    std::wstring input = L"ゐゑゔゕゖゝゞゟヰヱヿ";
    std::wstring correct = L"イエヴヵヶヽヾヨリイエコト";
    test_pronunciation_preprocessor(input, correct);
}

TEST_CASE( "parse a sentence including a conjugated form to text", "[language]" ) {
    std::wstring input = L"今日はもう出かけなきゃ";
    std::wstring correct = L"キョーハモウデカケナキャ";
    test_pronunciation_preprocessor(input, correct);
}

TEST_CASE( "parse a sentence including marks to text", "[language]" ) {
    std::wstring input = L"あっ!!今日何の日だっけ？教えて。";
    std::wstring correct = L"アッ!!キョーナンノヒダッケ？オシエテ。";
    test_pronunciation_preprocessor(input, correct);
}
