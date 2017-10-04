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

#include "romaji_indexer.hpp"

namespace resembla {

RomajiIndexer::RomajiIndexer(const std::string mecab_options, const size_t mecab_feature_pos,
        const std::string mecab_pronunciation_of_marks, bool keep_case):
    RomajiPreprocessor(mecab_options, mecab_feature_pos, mecab_pronunciation_of_marks), keep_case(keep_case) {}

string_type RomajiIndexer::index(const string_type& text) const
{
    auto r = RomajiPreprocessor::operator()(text);
    if(keep_case){
        return r;
    }

    string_type t;
    for(auto c: r){
        if(L'A' <= c && c <= L'Z'){
            c = c - L'A' + L'a';
        }
        t += c;
    }
    return t;
}

}
