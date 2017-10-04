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
            if((available = !ifs.eof())){
                this->operator++();
            }
        }

        iterator(): available(false){}

        bool operator!=(const iterator& i) const
        {
            return available || i.available;
        }

        iterator& operator++()
        {
            std::string raw_line;
            std::getline(ifs, raw_line);
            if(!(available = !ifs.eof())){
                return *this;
            }

            if(raw_line.empty()){
                return this->operator++();
            }

            auto line = cast_string<string_type>(raw_line);
            if(!comment_str.empty() && line.compare(0, 1, comment_str) == 0){
                return this->operator++();
            }

            columns = split(line, delimiter);
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

        bool available;
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
