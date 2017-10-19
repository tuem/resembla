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

#include <Catch/catch.hpp>
#include <json.hpp>

#include "string_util.hpp"

#include "measure/asis_preprocessor.hpp"
#include "measure/word_preprocessor.hpp"
#include "measure/romaji_preprocessor.hpp"
#include "measure/keyword_match_preprocessor.hpp"
#include "measure/weighted_sequence_serializer.hpp"

using namespace resembla;
using json = nlohmann::json;

TEST_CASE( "serialize and deserialize output data of asis sequence builder", "[serialization]" ) {
    init_locale();

    AsIsPreprocessor<string_type>::output_type o0 = L"テキスト!";

    json j0 = o0;
    const std::string s = j0.dump();
    CHECK(s == "[12486,12461,12473,12488,33]");

    json j1 = json::parse(s);
    AsIsPreprocessor<string_type>::output_type o1 = j1;
    CHECK(o1== o0);
}

TEST_CASE( "serialize and deserialize output data of keyword match preprocessor", "[serialization]" ) {
    init_locale();

    KeywordMatchPreprocessor<AsIsPreprocessor<string_type>>::output_type o0 = {L"テキスト!", {L"キーワード0", L"キーワード1"}};

    json j0 = o0;
    const std::string s = j0.dump();
    CHECK(s == "{\"k\":[\"キーワード0\",\"キーワード1\"],\"t\":\"テキスト!\"}");

    json j1 = json::parse(s);
    KeywordMatchPreprocessor<AsIsPreprocessor<string_type>>::output_type o1 = j1;
    CHECK(o1.text == o0.text);
    CHECK(o1.keywords == o0.keywords);
}

TEST_CASE( "serialize and deserialize output data of weighted word sequence builder", "[serialization]" ) {
    init_locale();

    WeightedSequenceBuilder<WordPreprocessor<string_type>, WordWeight>::output_type o0 = {{{L"単語0", {L"素性00", L"素性01"}}, 0.3}, {{L"単語1", {L"素性10", L"素性11"}}, 0.7}};

    json j0 = o0;
    const std::string s = j0.dump();
    CHECK(s == "[{\"t\":{\"f\":[\"素性00\",\"素性01\"],\"s\":\"単語0\"},\"w\":0.3},{\"t\":{\"f\":[\"素性10\",\"素性11\"],\"s\":\"単語1\"},\"w\":0.7}]");

    json j1 = json::parse(s);
    WeightedSequenceBuilder<WordPreprocessor<string_type>, WordWeight>::output_type o1 = j1;
    REQUIRE(o1.size() == o0.size());
    for(size_t i = 0; i < o1.size(); ++i){
        CHECK(o1[i].token.surface == o0[i].token.surface);
        CHECK(o1[i].token.feature == o0[i].token.feature);
        CHECK(o1[i].weight == o0[i].weight);
    }
}

TEST_CASE( "serialize and deserialize output data of weighted romaji sequence builder", "[serialization]" ) {
    init_locale();

    WeightedSequenceBuilder<RomajiPreprocessor, RomajiWeight>::output_type o0 = {{L'T', 0.3}, {L'e', 0.7}};

    json j0 = o0;
    const std::string s = j0.dump();
    CHECK(s == "[{\"t\":\"T\",\"w\":0.3},{\"t\":\"e\",\"w\":0.7}]");

    json j1 = json::parse(s);
    WeightedSequenceBuilder<RomajiPreprocessor, RomajiWeight>::output_type o1 = j1;
    REQUIRE(o1.size() == o0.size());
    for(size_t i = 0; i < o1.size(); ++i){
        CHECK(o1[i].token == o0[i].token);
        CHECK(o1[i].weight == o0[i].weight);
    }
}
