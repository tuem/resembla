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

#include "history.hpp"

#include <iomanip>

namespace resembla {

History::History()
{
    time_records.push_back({std::chrono::system_clock::now(), "", 0});
}

void History::record(const std::string& task, int count)
{
    time_records.push_back({std::chrono::system_clock::now(), task, count});
}

void History::dump(std::ostream& os) const
{
    os << "task\ttime[ms]\tcount\taverage[ms]" << std::endl;
    for(size_t i = 1; i < time_records.size(); ++i){
        auto t = std::chrono::duration_cast<std::chrono::microseconds>(
                time_records[i].time - time_records[i - 1].time).count() / 1000.0;
        os <<
            time_records[i].task << "\t" <<
            std::setprecision(10) << t << "\t" <<
            time_records[i].count << "\t" <<
            std::setprecision(10) << t / time_records[i].count << std::endl;
    }
}

}
