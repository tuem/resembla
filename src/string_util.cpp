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

#include <stdlib.h>

#include <locale>
#include <codecvt>
#include <iostream>

#include <unicode/unistr.h>

#include "string_util.hpp"

namespace resembla {

// common initialization procedures for using wchar_t
void init_locale()
{
    std::ios_base::sync_with_stdio(false);
    std::locale default_loc("");
    std::locale::global(std::locale(default_loc, new std::codecvt_utf8<wchar_t>));
    std::locale ctype_default_loc(std::locale::classic(), default_loc, std::locale::ctype);
    std::wcin.imbue(std::locale());
    std::wcout.imbue(ctype_default_loc);
    std::wcerr.imbue(ctype_default_loc);
    setlocale(LC_ALL, "");
}

template<>
void cast_string(const std::string& src, std::wstring& dest)
{
    std::wstring::value_type *wcs = new std::wstring::value_type[src.length() + 1];
    mbstowcs(wcs, src.c_str(), src.length() + 1);
    dest = wcs;
    delete [] wcs;
}

template<>
void cast_string(const std::wstring& src, std::string& dest)
{
    std::string::value_type *mbs = new std::string::value_type[src.length() * MB_CUR_MAX + 1];
    wcstombs(mbs, src.c_str(), src.length() * MB_CUR_MAX + 1);
    dest = mbs;
    delete [] mbs;
}

template<>
void cast_string(const std::string& src, UnicodeString& dest)
{
    dest = UnicodeString::fromUTF8(src);
}

template<>
void cast_string(const UnicodeString& src, std::string& dest)
{
    src.toUTF8String(dest);
}

template<>
void cast_string(const std::wstring& src, UnicodeString& dest)
{
    dest = UnicodeString::fromUTF8(cast_string<std::string>(src));
}

template<>
void cast_string(const UnicodeString& src, std::wstring& dest)
{
    std::string tmp;
    src.toUTF8String(tmp);
    cast_string(tmp, dest);
}

std::vector<std::string> split(std::string text, char delimiter)
{
    std::vector<std::string> result;
    for(size_t start = 0, end; start < text.length(); start = end + 1){
        end = text.find(delimiter, start);
        if(end == std::string::npos){
            end = text.length();
        }
        result.push_back(text.substr(start, end - start));
        start = end + 1;
    }
    return result;
}

template<> std::basic_ostream<char>& cio<char>::out = std::cout;
template<> std::basic_ostream<char>& cio<char>::err = std::cerr;
template<> std::basic_istream<char>& cio<char>::in = std::cin;

template<> std::basic_ostream<wchar_t>& cio<wchar_t>::out = std::wcout;
template<> std::basic_ostream<wchar_t>& cio<wchar_t>::err = std::wcerr;
template<> std::basic_istream<wchar_t>& cio<wchar_t>::in = std::wcin;

}
