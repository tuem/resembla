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

#ifndef RESEMBLA_STRING_UTIL_HPP
#define RESEMBLA_STRING_UTIL_HPP

#include <string>
#include <vector>
#include <unordered_map>

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

template<typename dest_type>
dest_type cast_string(const char* src)
{
    return cast_string<dest_type>(std::string(src));
}

template<typename char_type = char>
constexpr char_type column_delimiter();
// TODO: implement by a generic template function like this:
//    return cast_string<string_type>(std::string(1, '\t'))[0];

template<>
constexpr char column_delimiter()
{
    return '\t';
}

template<>
constexpr wchar_t column_delimiter()
{
    return L'\t';
}

template<typename char_type = char>
constexpr char_type attribute_delimiter();

template<>
constexpr char attribute_delimiter()
{
    return '&';
}

template<>
constexpr wchar_t attribute_delimiter()
{
    return L'&';
}

template<typename char_type = char>
constexpr char_type keyvalue_delimiter();

template<>
constexpr char keyvalue_delimiter()
{
    return '=';
}

template<>
constexpr wchar_t keyvalue_delimiter()
{
    return L'=';
}

template<typename char_type = char>
constexpr char_type value_delimiter();

template<>
constexpr char value_delimiter()
{
    return ',';
}

template<>
constexpr wchar_t value_delimiter()
{
    return L',';
}

template<typename char_type = char>
constexpr char_type comment_prefix();

template<>
constexpr char comment_prefix()
{
    return '#';
}

template<>
constexpr wchar_t comment_prefix()
{
    return L'#';
}

template<typename string_type>
std::vector<string_type> split(const string_type& text,
        typename string_type::value_type delimiter = column_delimiter<typename string_type::value_type>(),
        size_t max = 0)
{
    std::vector<string_type> result;
    bool finished = false;
    for(size_t start = 0, end; start < text.length();){
        if(max == 0 || result.size() + 1 < max){
            end = text.find(delimiter, start);
            if(end == string_type::npos){
                end = text.length();
                finished = true;
            }
        }
        else{
            end = text.length();
            finished = true;
        }
        result.push_back(text.substr(start, end - start));
        start = end + 1;
    }
    if(!finished){
        result.push_back(string_type());
    }
    return result;
}

template<typename string_type>
std::unordered_map<string_type, string_type> splitToKeyValueMap(const string_type& text,
        typename string_type::value_type delim = attribute_delimiter<typename string_type::value_type>(),
        typename string_type::value_type delim_kv = keyvalue_delimiter<typename string_type::value_type>())
{
    std::unordered_map<string_type, string_type> result;
    if(!text.empty()){
        for(const auto& s: split<string_type>(text, delim)){
            auto kv = split<string_type>(s, delim_kv, 2);
            if(kv.size() == 2){
                result[kv[0]] = kv[1];
            }
        }
    }
    return result;
}

}
#endif
