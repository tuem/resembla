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

#ifndef RESEMBLA_CSV_READER_HPP
#define RESEMBLA_CSV_READER_HPP

#include <string>
#include <vector>
#include <fstream>

#include "string_util.hpp"

namespace resembla {

template<typename string_type = std::string>
class CsvReader
{
public:
    using symbol_type = typename string_type::value_type;

    CsvReader(const std::string& file_path, size_t min_columns = 0,
            symbol_type delimiter = column_delimiter<symbol_type>(),
            symbol_type comment_symbol = comment_prefix<symbol_type>()):
        file_path(file_path), min_columns(min_columns),
        delimiter(delimiter), comment_symbol(comment_symbol)
    {}

    class iterator
    {
    public:
        using value_type = std::vector<string_type>;

        iterator(const std::string& file_path, size_t min_columns,
                symbol_type delimiter, symbol_type comment_symbol):
            ifs(file_path), min_columns(min_columns), delimiter(delimiter),
            comment_str(comment_symbol == static_cast<symbol_type>(0) ?
                    string_type(0) : string_type(1, comment_symbol))
        {
            if(ifs.fail()){
                throw std::runtime_error("input file is not available: " + file_path);
            }
            eof = ifs.eof();
            if(!eof){
                this->operator++();
            }
        }

        iterator(): eof(true){}

        bool operator!=(const iterator& i) const
        {
            return !eof || !i.eof;
        }

        iterator& operator++()
        {
            std::string line;
            std::getline(ifs, line);
            if(ifs.eof()){
                eof = true;
                columns.clear();
                return *this;
            }

            if(line.empty()){
                return this->operator++();
            }

            string_type _line = cast_string<string_type>(line);
            if(!comment_str.empty() && _line.compare(0, 1, comment_str) == 0){
                return this->operator++();
            }

            columns = split(_line, delimiter);
            if(columns.size() < min_columns){
                return this->operator++();
            }

            return *this;
        }

        const value_type& operator*() const
        {
            return columns;
        }

    protected:
        std::ifstream ifs;
        size_t min_columns;
        symbol_type delimiter;
        string_type comment_str;

        bool eof;
        std::vector<string_type> columns;
    };

    iterator begin() const
    {
        return iterator(file_path, min_columns, delimiter, comment_symbol);
    }

    iterator end() const
    {
        return iterator();
    }

protected:
    std::string file_path;
    size_t min_columns;
    symbol_type delimiter;
    symbol_type comment_symbol;
};

}
#endif
