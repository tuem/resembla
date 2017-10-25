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

#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <set>
#include <memory>
#include <stdexcept>

#include "mecab_util.hpp"

using namespace resembla;

TEST_CASE( "validate MeCab options", "[MeCab]" ) {
    REQUIRE_NOTHROW(validate_mecab_options(""));
    REQUIRE_NOTHROW(validate_mecab_options("-Odump  "));
    REQUIRE_NOTHROW(validate_mecab_options("-d ./dummy_dict "));
    REQUIRE_NOTHROW(validate_mecab_options("--dicdir ./dummy_dict "));
    REQUIRE_NOTHROW(validate_mecab_options("-Odump  -d dummy_dict"));
    REQUIRE_NOTHROW(validate_mecab_options("-d  ./dummy_dict/ -Odump"));
    REQUIRE_THROWS(validate_mecab_options("-Odump -d /invalid_dict"));
    REQUIRE_THROWS(validate_mecab_options("--dicdir ./invalid_dict -Odump"));
    REQUIRE_THROWS(validate_mecab_options("-d"));
    REQUIRE_THROWS(validate_mecab_options("-Odump -d"));
}
