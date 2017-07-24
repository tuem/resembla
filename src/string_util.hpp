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

#ifndef __STRING_UTIL_HPP__
#define __STRING_UTIL_HPP__

#include <string>
#include <vector>

namespace resembla {

using string_type = std::wstring;

// common initialization procedures for using wchar_t
void init_locale();

template<typename src_type, typename dest_type>
void cast_string(const src_type& src, dest_type& dest);

template<typename src_type>
void cast_string(const src_type& src, src_type& dest)
{
    dest = src;
}

template<typename dest_type, typename src_type>
dest_type cast_string(const src_type& src)
{
    dest_type desc;
    cast_string(src, desc);
    return desc;
}

// split text by delimiter
std::vector<std::string> split(std::string text, char delimiter = ',');

}
#endif
