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
#include <memory>
#include <mutex>

#include <simstring/simstring.h>

#include "eliminator.hpp"

namespace resembla {

template<typename Indexer>
class SimStringDatabase
{
public:
    using string_type = Indexer::string_type;

    SimStringDatabase(const std::string& db_path, int measure, double threshold,
            std::shared_ptr<Indexer> index):
        measure(measure), threshold(threshold), index(index)
    {
        db.open(db_path);
    }

    std::vector<string_type> search(const string_type& query, size_t max_output = 0) const
    {
        auto search_query = index(query);
        std::vector<string_type> result;
        {
            std::lock_guard<std::mutex> lock(mutex_simstring);
            db.retrieve(search_query, measure, threshold,
                    std::back_inserter(result));
        }
        if(result.empty()){
            return {};
        }
        else if(max_output != 0 && result.size() > max_output){
            Eliminator<string_type> eliminate(search_query);
            eliminate(result, max_output);
        }

        return result;
    }

protected:
    mutable simstring::reader db;
    mutable std::mutex mutex_simstring;

    const int measure;
    const double threshold;

    const std::shared_ptr<Indexer> index;
};

}
#endif
