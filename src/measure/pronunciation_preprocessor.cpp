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

#include "pronunciation_preprocessor.hpp"

#include <vector>

#include "../string_util.hpp"
#include "../mecab_util.hpp"

namespace resembla {

const std::unordered_map<PronunciationPreprocessor::token_type, string_type>
        PronunciationPreprocessor::KANA_MAP = {
    {L'ぁ', L"ァ"},
    {L'あ', L"ア"},
    {L'ぃ', L"ィ"},
    {L'い', L"イ"},
    {L'ぅ', L"ゥ"},
    {L'う', L"ウ"},
    {L'ぇ', L"ェ"},
    {L'え', L"エ"},
    {L'ぉ', L"ォ"},
    {L'お', L"オ"},
    {L'か', L"カ"},
    {L'が', L"ガ"},
    {L'き', L"キ"},
    {L'ぎ', L"ギ"},
    {L'く', L"ク"},
    {L'ぐ', L"グ"},
    {L'け', L"ケ"},
    {L'げ', L"ゲ"},
    {L'こ', L"コ"},
    {L'ご', L"ゴ"},
    {L'さ', L"サ"},
    {L'ざ', L"ザ"},
    {L'し', L"シ"},
    {L'じ', L"ジ"},
    {L'す', L"ス"},
    {L'ず', L"ズ"},
    {L'せ', L"セ"},
    {L'ぜ', L"ゼ"},
    {L'そ', L"ソ"},
    {L'ぞ', L"ゾ"},
    {L'た', L"タ"},
    {L'だ', L"ダ"},
    {L'ち', L"チ"},
    {L'ぢ', L"ヂ"},
    {L'っ', L"ッ"},
    {L'つ', L"ツ"},
    {L'づ', L"ヅ"},
    {L'て', L"テ"},
    {L'で', L"デ"},
    {L'と', L"ト"},
    {L'ど', L"ド"},
    {L'な', L"ナ"},
    {L'に', L"ニ"},
    {L'ぬ', L"ヌ"},
    {L'ね', L"ネ"},
    {L'の', L"ノ"},
    {L'は', L"ハ"},
    {L'ば', L"バ"},
    {L'ぱ', L"パ"},
    {L'ひ', L"ヒ"},
    {L'び', L"ビ"},
    {L'ぴ', L"ピ"},
    {L'ふ', L"フ"},
    {L'ぶ', L"ブ"},
    {L'ぷ', L"プ"},
    {L'へ', L"ヘ"},
    {L'べ', L"ベ"},
    {L'ぺ', L"ペ"},
    {L'ほ', L"ホ"},
    {L'ぼ', L"ボ"},
    {L'ぽ', L"ポ"},
    {L'ま', L"マ"},
    {L'み', L"ミ"},
    {L'む', L"ム"},
    {L'め', L"メ"},
    {L'も', L"モ"},
    {L'ゃ', L"ャ"},
    {L'や', L"ヤ"},
    {L'ゅ', L"ュ"},
    {L'ゆ', L"ユ"},
    {L'ょ', L"ョ"},
    {L'よ', L"ヨ"},
    {L'ら', L"ラ"},
    {L'り', L"リ"},
    {L'る', L"ル"},
    {L'れ', L"レ"},
    {L'ろ', L"ロ"},
    {L'ゎ', L"ヮ"},
    {L'わ', L"ワ"},
    {L'ゐ', L"イ"},
    {L'ゑ', L"エ"},
    {L'を', L"ヲ"},
    {L'ん', L"ン"},
    {L'ゔ', L"ヴ"},
    {L'ゕ', L"ヵ"},
    {L'ゖ', L"ヶ"},
    {L'ゟ', L"ヨリ"},
    {L'ゝ', L"ヽ"},
    {L'ゞ', L"ヾ"},
    {L'ァ', L"ァ"},
    {L'ア', L"ア"},
    {L'ィ', L"ィ"},
    {L'イ', L"イ"},
    {L'ゥ', L"ゥ"},
    {L'ウ', L"ウ"},
    {L'ェ', L"ェ"},
    {L'エ', L"エ"},
    {L'ォ', L"ォ"},
    {L'オ', L"オ"},
    {L'カ', L"カ"},
    {L'ガ', L"ガ"},
    {L'キ', L"キ"},
    {L'ギ', L"ギ"},
    {L'ク', L"ク"},
    {L'グ', L"グ"},
    {L'ケ', L"ケ"},
    {L'ゲ', L"ゲ"},
    {L'コ', L"コ"},
    {L'ゴ', L"ゴ"},
    {L'サ', L"サ"},
    {L'ザ', L"ザ"},
    {L'シ', L"シ"},
    {L'ジ', L"ジ"},
    {L'ス', L"ス"},
    {L'ズ', L"ズ"},
    {L'セ', L"セ"},
    {L'ゼ', L"ゼ"},
    {L'ソ', L"ソ"},
    {L'ゾ', L"ゾ"},
    {L'タ', L"タ"},
    {L'ダ', L"ダ"},
    {L'チ', L"チ"},
    {L'ヂ', L"ヂ"},
    {L'ッ', L"ッ"},
    {L'ツ', L"ツ"},
    {L'ヅ', L"ヅ"},
    {L'テ', L"テ"},
    {L'デ', L"デ"},
    {L'ト', L"ト"},
    {L'ド', L"ド"},
    {L'ナ', L"ナ"},
    {L'ニ', L"ニ"},
    {L'ヌ', L"ヌ"},
    {L'ネ', L"ネ"},
    {L'ノ', L"ノ"},
    {L'ハ', L"ハ"},
    {L'バ', L"バ"},
    {L'パ', L"パ"},
    {L'ヒ', L"ヒ"},
    {L'ビ', L"ビ"},
    {L'ピ', L"ピ"},
    {L'フ', L"フ"},
    {L'ブ', L"ブ"},
    {L'プ', L"プ"},
    {L'ヘ', L"ヘ"},
    {L'ベ', L"ベ"},
    {L'ペ', L"ペ"},
    {L'ホ', L"ホ"},
    {L'ボ', L"ボ"},
    {L'ポ', L"ポ"},
    {L'マ', L"マ"},
    {L'ミ', L"ミ"},
    {L'ム', L"ム"},
    {L'メ', L"メ"},
    {L'モ', L"モ"},
    {L'ャ', L"ャ"},
    {L'ヤ', L"ヤ"},
    {L'ュ', L"ュ"},
    {L'ユ', L"ユ"},
    {L'ョ', L"ョ"},
    {L'ヨ', L"ヨ"},
    {L'ラ', L"ラ"},
    {L'リ', L"リ"},
    {L'ル', L"ル"},
    {L'レ', L"レ"},
    {L'ロ', L"ロ"},
    {L'ヮ', L"ヮ"},
    {L'ワ', L"ワ"},
    {L'ヰ', L"イ"},
    {L'ヱ', L"エ"},
    {L'ヲ', L"ヲ"},
    {L'ン', L"ン"},
    {L'ヴ', L"ヴ"},
    {L'ヵ', L"ヵ"},
    {L'ヶ', L"ヶ"},
    {L'ヿ', L"コト"},
};

bool PronunciationPreprocessor::isKanaWord(const string_type& w) const
{
    for(const auto c: w){
        if(KANA_MAP.find(c) == KANA_MAP.end()){
            return false;
        }
    }
    return true;
}

string_type PronunciationPreprocessor::estimatePronunciation(const string_type& w) const
{
    string_type y;
    for(auto c: w){
        if(KANA_MAP.find(c) == KANA_MAP.end()){
            y.push_back(c);
        }
        else{
            const auto& katakana = KANA_MAP.at(c);
            for(auto d: katakana){
                if(KANA_MAP.find(d) == KANA_MAP.end()){
                    y.push_back(d);
                }
                else{
                    y += KANA_MAP.at(d);
                }
            }
        }
    }
    return y;
}

PronunciationPreprocessor::PronunciationPreprocessor(
        const std::string& mecab_options, size_t mecab_feature_pos,
        const std::string& mecab_pronunciation_of_marks):
    tagger(MeCab::createTagger(validate_mecab_options(mecab_options).c_str())),
    mecab_feature_pos(mecab_feature_pos),
    mecab_pronunciation_of_marks(cast_string<string_type>(mecab_pronunciation_of_marks))
{}

PronunciationPreprocessor::output_type PronunciationPreprocessor::operator()(
        const string_type& text, bool is_original) const
{
    std::string text_string = cast_string<std::string>(
            is_original ? split(text, column_delimiter<string_type::value_type>())[0] : text);
    output_type s;
    {
        std::lock_guard<std::mutex> lock(mutex_tagger);
        for(const MeCab::Node* node = tagger->parseToNode(text_string.c_str()); node; node = node->next){
            // skip BOS/EOS nodes
            if(node->stat == MECAB_BOS_NODE || node->stat == MECAB_EOS_NODE){
                continue;
            }

            // TODO: improve efficiency
            // extract surface and features
            string_type surface = cast_string<string_type>(std::string(node->surface, node->surface + node->length));
            std::vector<string_type> feature;
            const char *start = node->feature;
            for(const char* end = start; *end != '\0'; ++end){
                if(*end == ','){
                    if(start < end){
                        feature.push_back(cast_string<string_type>(std::string(start, end)));
                    }
                    start = end + 1;
                }
            }
            if(*start != '\0'){
                feature.push_back(cast_string<string_type>(std::string(start)));
            }
            while(feature.size() <= mecab_feature_pos){
                feature.push_back(string_type());
            }

            // extract surface and features
            string_type pronunciation;
            if(feature[mecab_feature_pos].empty() || feature[mecab_feature_pos] == L"*" || isKanaWord(surface)){
                pronunciation = estimatePronunciation(surface);
            }
            else if(feature[mecab_feature_pos] == mecab_pronunciation_of_marks){
                pronunciation = surface;
            }
            else{
                // convert old katakanas
                for(auto c: feature[mecab_feature_pos]){
                    if(KANA_MAP.find(c) == KANA_MAP.end()){
                        pronunciation.push_back(c);
                    }
                    else{
                        pronunciation += KANA_MAP.at(c);
                    }
                }
            }

            for(auto c: pronunciation){
                s.push_back(c);
            }
        }
    }
    return s;
}

}
