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
#include "measure/edit_distance.hpp"

using namespace resembla;

TEST_CASE( "compute edit distance", "[measure]" ) {
    init_locale();
    EditDistance<> ed;
    CHECK(ed(std::string(""), std::string("")) == Approx(1.0));
    CHECK(ed(std::string("a"), std::string("")) == Approx(0.0));
    CHECK(ed(std::string(""), std::string("xyz")) == Approx(0.0));
    CHECK(ed(std::string("abc"), std::string("abc")) == Approx(1.0));
    CHECK(ed(std::string("abc"), std::string("axc")) == Approx(4.0 / 6));
    CHECK(ed(std::string("abc"), std::string("ab")) == Approx(4.0 / 5));
    CHECK(ed(std::string("bc"), std::string("abc")) == Approx(4.0 / 5));
    CHECK(ed(std::string("ad"), std::string("abcd")) == Approx(4.0 / 6));
    CHECK(ed(std::string("abce"), std::string("bxde")) == Approx(4.0 / 8));
}
