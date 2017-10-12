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

#include <iostream>
#include <fstream>
#include <unordered_set>
#include <chrono>
#include <time.h>

#include <paramset.hpp>

#include "eliminator.hpp"
#include "string_util.hpp"

#include "history.hpp"

using namespace resembla;

int main(int argc, char* argv[])
{
    History history;
    init_locale();

    paramset::definitions defs = {
        {"col", 0, {"col"}, "col", 'i', "column number of text in tab-separated lines. use whole string of line if col=0"},
        {"repeat", 0, {"repeat"}, "repeat", 'r', "repeat count of elimination process"},
        {"conf_path", "", "config", 'c', "config file path"}
    };
    paramset::manager pm(defs);
    try{
        pm.load(argc, argv, "config");
        std::string path = pm.rest.size() > 0 ? pm.rest[0] : "";
        size_t col = pm.get<int>("col");
        size_t repeat = pm.get<int>("repeat");

        std::vector<string_type> texts;
        std::istream* is = path.empty() ? &std::cin : new std::ifstream(path);
        while(is->good()){
            std::string line;
            std::getline(*is, line);
            if(is->eof()){
                break;
            }
            else if(line.empty()){
                continue;
            }

            string_type text;
            if(col == 0){
                texts.push_back(cast_string<string_type>(line));
            }
            else{
                auto columns = split(line, column_delimiter<>());
                if(col - 1 < columns.size()){
                    texts.push_back(cast_string<string_type>(columns[col - 1]));
                }
            }
        }
        if(is != &std::cin){
            delete is;
        }

        std::unordered_set<typename string_type::value_type> alphabet;
        size_t total_length = 0;
        for(const auto& text: texts){
            total_length += text.length();
            for(auto c: text){
                alphabet.insert(c);
            }
        }
        std::cout << "corpus size: " << texts.size() << std::endl;
        std::cout << "alphabet size: " << alphabet.size() << std::endl;
        std::cout << "avarage length: " << total_length / static_cast<double>(texts.size()) << std::endl;
        history.record("loading", 1);

        std::vector<Eliminator<string_type>> eliminators;
        for(size_t i = 0; i < repeat; ++i){
            eliminators.push_back({texts[i % texts.size()]});
        }
        history.record("preprocess", repeat);

        for(auto& eliminate: eliminators){
            eliminate(texts, texts.size());
        }
        history.record("elimination", repeat);
    }
    catch(const std::exception& e){
        std::cerr << "error: " << e.what() << std::endl;
        exit(1);
    }

    history.dump(std::cout, true, true);

    return 0;
}
