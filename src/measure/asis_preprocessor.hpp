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

#ifndef RESEMBLA_ASIS_PREPROCESSOR_HPP
#define RESEMBLA_ASIS_PREPROCESSOR_HPP

#include "../string_util.hpp"

namespace resembla {

template<typename String>
struct AsIsPreprocessor
{
    using string_type = String;
    using token_type = typename String::value_type;
    using output_type = String;

    output_type operator()(const String& text, bool is_original = false) const
    {
        return is_original ? split(text, column_delimiter<token_type>())[0] : text;
    }
};

}
#endif
