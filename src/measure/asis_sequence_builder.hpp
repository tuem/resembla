/*
Resembla: Word-based Japanese similar sentence search library
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

#ifndef __ASIS_SEQUENCE_BUILDER_HPP__
#define __ASIS_SEQUENCE_BUILDER_HPP__

#include <string>

namespace resembla {

template<typename string_type>
class AsIsSequenceBuilder
{
public:
    using token_type = typename string_type::value_type;
    using output_type = string_type;

    output_type build(const string_type& text, bool) const
    {
        return text;
    }

    string_type buildIndexingText(const string_type& text) const
    {
        return text;
    }
};

}
#endif
