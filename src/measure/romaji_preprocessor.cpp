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

#include "romaji_preprocessor.hpp"

namespace resembla {

const std::unordered_map<string_type, string_type> RomajiPreprocessor::ROMAJI_MAP = {
    {L"ァ", L"a"},
    {L"ア", L"A"},
    {L"ィ", L"i"},
    {L"イ", L"I"},
    {L"ゥ", L"u"},
    {L"ウ", L"U"},
    {L"ェ", L"e"},
    {L"エ", L"E"},
    {L"ォ", L"o"},
    {L"オ", L"O"},
    {L"カ", L"KA"},
    {L"ガ", L"GA"},
    {L"キ", L"KI"},
    {L"ギ", L"GI"},
    {L"ク", L"KU"},
    {L"グ", L"GU"},
    {L"ケ", L"KE"},
    {L"ゲ", L"GE"},
    {L"コ", L"KO"},
    {L"ゴ", L"GO"},
    {L"サ", L"SA"},
    {L"ザ", L"ZA"},
    {L"シ", L"SI"},
    {L"ジ", L"ZI"},
    {L"ス", L"SU"},
    {L"ズ", L"ZU"},
    {L"セ", L"SE"},
    {L"ゼ", L"ZE"},
    {L"ソ", L"SO"},
    {L"ゾ", L"ZO"},
    {L"タ", L"TA"},
    {L"ダ", L"DA"},
    {L"チ", L"TI"},
    {L"ヂ", L"DI"},
    {L"ッ", L"tu"},
    {L"ツ", L"TU"},
    {L"ヅ", L"DU"},
    {L"テ", L"TE"},
    {L"デ", L"DE"},
    {L"ト", L"TO"},
    {L"ド", L"DO"},
    {L"ナ", L"NA"},
    {L"ニ", L"NI"},
    {L"ヌ", L"NU"},
    {L"ネ", L"NE"},
    {L"ノ", L"NO"},
    {L"ハ", L"HA"},
    {L"バ", L"BA"},
    {L"パ", L"PA"},
    {L"ヒ", L"HI"},
    {L"ビ", L"BI"},
    {L"ピ", L"PI"},
    {L"フ", L"HU"},
    {L"ブ", L"BU"},
    {L"プ", L"PU"},
    {L"ヘ", L"HE"},
    {L"ベ", L"BE"},
    {L"ペ", L"PE"},
    {L"ホ", L"HO"},
    {L"ボ", L"BO"},
    {L"ポ", L"PO"},
    {L"マ", L"MA"},
    {L"ミ", L"MI"},
    {L"ム", L"MU"},
    {L"メ", L"ME"},
    {L"モ", L"MO"},
    {L"ャ", L"ya"},
    {L"ヤ", L"YA"},
    {L"ュ", L"yu"},
    {L"ユ", L"YU"},
    {L"ョ", L"yo"},
    {L"ヨ", L"YO"},
    {L"ラ", L"RA"},
    {L"リ", L"RI"},
    {L"ル", L"RU"},
    {L"レ", L"RE"},
    {L"ロ", L"RO"},
    {L"ヮ", L"wa"},
    {L"ワ", L"WA"},
    {L"ヲ", L"WO"},
    {L"ン", L"n"},
    {L"ヴァ", L"VA"},
    {L"ヴィ", L"VI"},
    {L"ヴ", L"VU"},
    {L"ヴェ", L"VE"},
    {L"ヴォ", L"VO"},
    {L"ヵ", L"ka"},
    {L"ヶ", L"ke"},
    {L"ー", L"-"},
    {L"キャ", L"Kya"},
    {L"ギャ", L"Gya"},
    {L"キュ", L"Kyu"},
    {L"ギュ", L"Gyu"},
    {L"キョ", L"Kyo"},
    {L"ギョ", L"Gyo"},
    {L"シャ", L"Sya"},
    {L"ジャ", L"Zya"},
    {L"シュ", L"Syu"},
    {L"ジュ", L"Zyu"},
    {L"ショ", L"Syo"},
    {L"ジョ", L"Zyo"},
    {L"チャ", L"Tya"},
    {L"ヂャ", L"Dya"},
    {L"チュ", L"Tyu"},
    {L"ヂュ", L"Dyu"},
    {L"チョ", L"Tyo"},
    {L"ヂョ", L"Dyo"},
    {L"ニャ", L"Nya"},
    {L"ニュ", L"Nyu"},
    {L"ニョ", L"Nyo"},
    {L"ヒャ", L"Hya"},
    {L"ビャ", L"Bya"},
    {L"ピャ", L"Pya"},
    {L"ヒュ", L"Hyu"},
    {L"ビュ", L"Byu"},
    {L"ピュ", L"Pyu"},
    {L"ヒョ", L"Hyo"},
    {L"ビョ", L"Byo"},
    {L"ピョ", L"Pyo"},
    {L"ミャ", L"Mya"},
    {L"ミュ", L"Myu"},
    {L"ミョ", L"Myo"},
    {L"リャ", L"Rya"},
    {L"リュ", L"Ryu"},
    {L"リョ", L"Ryo"},
    {L"クヮ", L"Kwa"},
    {L"グヮ", L"Gwa"},
    {L"ウィ", L"ui"},
    {L"ウェ", L"ue"},
    {L"ウォ", L"uo"},
    {L"チェ", L"Tie"},
    {L"ティ", L"Tei"},
    {L"ファ", L"Hua"},
    {L"フィ", L"Hui"},
    {L"フェ", L"Hue"},
    {L"フォ", L"Huo"},
    {L"ッカ", L"kKA"},
    {L"ッガ", L"gGA"},
    {L"ッキ", L"kKI"},
    {L"ッギ", L"gGI"},
    {L"ック", L"kKU"},
    {L"ッグ", L"gGU"},
    {L"ッケ", L"kKE"},
    {L"ッゲ", L"gGE"},
    {L"ッコ", L"kKO"},
    {L"ッゴ", L"gGO"},
    {L"ッサ", L"sSA"},
    {L"ッザ", L"zZA"},
    {L"ッシ", L"sSI"},
    {L"ッジ", L"zZI"},
    {L"ッス", L"sSU"},
    {L"ッズ", L"zZU"},
    {L"ッセ", L"sSE"},
    {L"ッゼ", L"zZE"},
    {L"ッソ", L"sSO"},
    {L"ッゾ", L"zZO"},
    {L"ッタ", L"tTA"},
    {L"ッダ", L"dDA"},
    {L"ッチ", L"tTI"},
    {L"ッヂ", L"dDI"},
    {L"ッツ", L"tTU"},
    {L"ッヅ", L"dDU"},
    {L"ッテ", L"tTE"},
    {L"ッデ", L"dDE"},
    {L"ット", L"tTO"},
    {L"ッド", L"dDO"},
    {L"ッナ", L"nNA"},
    {L"ッニ", L"nNI"},
    {L"ッヌ", L"nNU"},
    {L"ッネ", L"nNE"},
    {L"ッノ", L"nNO"},
    {L"ッハ", L"hHA"},
    {L"ッバ", L"bBA"},
    {L"ッパ", L"pPA"},
    {L"ッヒ", L"hHI"},
    {L"ッビ", L"bBI"},
    {L"ッピ", L"pPI"},
    {L"ッフ", L"hHU"},
    {L"ッブ", L"bBU"},
    {L"ップ", L"pPU"},
    {L"ッヘ", L"hHE"},
    {L"ッベ", L"bBE"},
    {L"ッペ", L"pPE"},
    {L"ッホ", L"hHO"},
    {L"ッボ", L"bBO"},
    {L"ッポ", L"pPO"},
    {L"ッマ", L"mMA"},
    {L"ッミ", L"mMI"},
    {L"ッム", L"mMU"},
    {L"ッメ", L"mME"},
    {L"ッモ", L"mMO"},
    {L"ッヤ", L"yYA"},
    {L"ッユ", L"yYU"},
    {L"ッヨ", L"yYO"},
    {L"ッラ", L"rRA"},
    {L"ッリ", L"rRI"},
    {L"ッル", L"rRU"},
    {L"ッレ", L"rRE"},
    {L"ッロ", L"rRO"},
    {L"ッワ", L"wWA"},
    {L"ッヲ", L"wWO"},
    {L"ッヴ", L"vVU"},
};

RomajiPreprocessor::RomajiPreprocessor(const std::string& mecab_options, size_t mecab_feature_pos,
        const std::string& mecab_pronunciation_of_marks, bool keep_case):
    PronunciationPreprocessor(mecab_options, mecab_feature_pos, mecab_pronunciation_of_marks),
    keep_case(keep_case)
{}

RomajiPreprocessor::output_type RomajiPreprocessor::operator()(const string_type& text, bool) const
{
    output_type s;
    auto pronunciation = PronunciationPreprocessor::operator()(text);
    for(size_t i = 0; i < pronunciation.size(); ++i){
        string_type p = {pronunciation[i]};
        // try to read-ahead next letter
        if(i + 1 < pronunciation.size()){
            auto q = p + pronunciation[i + 1];
            // if concatenated string exists in map, they can be converted to romaji at once
            if(ROMAJI_MAP.find(q) != ROMAJI_MAP.end()){
                if(p == L"ッ"){
                    // only process the first letter 'ッ' because pronunciation[i+1..i+2] may be able to concatenate
                    p = {ROMAJI_MAP.at(q)[0]};
                }
                else{
                    p = q;
                    ++i;
                }
            }
        }

        // convert to romaji if p exists in map
        if(ROMAJI_MAP.count(p) > 0){
            p = ROMAJI_MAP.at(p);
        }

        // put each roman alphabet to output sequence
        for(auto c: p){
            if(!keep_case && L'A' <= c && c <= L'Z'){
                c = c - L'A' + L'a';
            }
            s.push_back(c);
        }
    }
    return s;
}

}
