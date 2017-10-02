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

using namespace resembla;

class History final
{
public:
    History()
    {
        time_records.push_back({std::chrono::system_clock::now(), "", 0});
    }

    void record(const std::string& task, int count = 1)
    {
        time_records.push_back({std::chrono::system_clock::now(), task, count});
    }

    void dump(std::ostream& os = std::cout)
    {
        os << "task\ttime[ms]\tcount\taverage[ms]" << std::endl;
        for(size_t i = 1; i < time_records.size(); ++i){
            auto t = std::chrono::duration_cast<std::chrono::microseconds>(time_records[i].time - time_records[i - 1].time).count() / 1000.0;
            os <<
                time_records[i].task << "\t" <<
                std::setprecision(10) << t << "\t" <<
                time_records[i].count << "\t" <<
                std::setprecision(10) << t / time_records[i].count <<
                std::endl;
        }
    }

private:
    struct TimeRecord
    {
        std::chrono::system_clock::time_point time;
        std::string task;
        int count;
    };
    std::vector<TimeRecord> time_records;
};

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

    std::cout << std::endl;
    history.dump();

    return 0;
}
