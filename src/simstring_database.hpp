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

#ifndef RESEMBLA_SIMSTRING_DATABASE_HPP
#define RESEMBLA_SIMSTRING_DATABASE_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>

#include <simstring/simstring.h>

#include "eliminator.hpp"

namespace resembla {

template<typename Indexer>
class SimStringDatabase
{
public:
    using string_type = Indexer::output_type;

    SimStringDatabase(const std::string& simstring_db_path, int measure, double threshold,
            std::shared_ptr<Indexer> index, const std::string& inverse_path):
        measure(measure), threshold(threshold), index(index)
    {
        db.open(simstring_db_path);

        for(const auto& columns: CsvReader<string_type>(inverse_path, 2)){
            const auto& indexed = columns[0];
            const auto& original = columns[1];

            const auto& p = inverse.insert(std::pair<string_type, std::vector<string_type>>(indexed, {original}));
            if(!p.second){
                p.first->second.push_back(original);
            }
        }
    }

    std::vector<string_type> search(const string_type& query, size_t max_output = 0) const
    {
        auto search_query = index(query);

        std::vector<string_type> simstring_result;
        {
            std::lock_guard<std::mutex> lock(mutex_simstring);
            db.retrieve(search_query, measure, threshold,
                    std::back_inserter(simstring_result));
        }
        if(simstring_result.empty()){
            return {};
        }

        if(max_output != 0 && simstring_result.size() > max_output){
            Eliminator<string_type> eliminate(search_query);
            eliminate(result, max_output);
        }

        std::vector<string_type> result;
        for(const auto& i: simstring_result){
            if(i.empty()){
                continue;
            }
            const auto& j = inverse.at(i);
            std::copy(std::begin(j), std::end(j), std::back_inserter(result));
        }

        return result;
    }

    // TODO: implement database writer

protected:
    mutable simstring::reader db;
    mutable std::mutex mutex_simstring;

    const int measure;
    const double threshold;

    const std::shared_ptr<Indexer> index;

    std::unordered_map<string_type, std::vector<string_type>> inverse;
};

}
#endif
