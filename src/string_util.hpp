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

#ifndef RESEMBLA_STRING_UTIL_HPP
#define RESEMBLA_STRING_UTIL_HPP

#include <string>
#include <vector>
#include <iostream>

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

// TODO: use constexpr
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
constexpr char_type feature_delimiter();

template<>
constexpr char feature_delimiter()
{
    return '&';
}

template<>
constexpr wchar_t feature_delimiter()
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

// split text by delimiter
template<typename string_type>
std::vector<string_type> split(const string_type& text, const typename string_type::value_type delimiter)
{
    std::vector<string_type> result;
    bool finished = false;
    for(size_t start = 0, end; start < text.length();){
        end = text.find(delimiter, start);
        if(end == string_type::npos){
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

template<class string_type>
void narrow_down_by_unigram_intersection(const string_type& reference, std::vector<string_type>& target, size_t k)
{
    string_type a = reference;
    std::sort(std::begin(a), std::end(a));

    size_t max_length = 0;
    for(const auto& t: target){
        max_length = std::max(max_length, t.length());
    }
    string_type b(max_length, static_cast<typename string_type::value_type>(0));

    std::vector<std::pair<string_type, double>> work(target.size());
    size_t p = 0;
    for(const auto& t: target){
        std::copy(std::begin(t), std::end(t), std::begin(b));
        std::sort(std::begin(b), std::begin(b) + t.length());
        size_t total = a.length() + t.length(), i = 0, j = 0, c = 0;
        while(i < a.length() && j < t.length()){
            if(a[i] == b[j]){
                ++i;
                ++j;
                c += 2;
            }
            else if(a[i] < b[j]){
                ++i;
            }
            else{
                ++j;
            }
        }
        work[p].first = t;
        work[p++].second = c / static_cast<double>(total);
    }
    std::nth_element(std::begin(work), std::begin(work) + k, std::end(work),
        [](const std::pair<string_type, double>& a, const std::pair<string_type, double>& b) -> bool{
            return a.second > b.second;
        });
#ifdef DEBUG
    std::cerr << "narrow " << work.size() << " strings" << std::endl;
    for(const auto& i: work){
        std::cerr << cast_string<std::string>(i.first) << ": " << i.second << std::endl;
    }
#endif
    for(size_t i = 0; i < k; ++i){
        target[i] = work.at(i).first;
    }
    target.erase(std::begin(target) + k, std::end(target));
}

}
#endif
