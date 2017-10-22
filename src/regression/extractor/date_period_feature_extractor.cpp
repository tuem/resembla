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

#include "date_period_feature_extractor.hpp"

#include <ctime>
#include <sstream>
#include <iomanip>

namespace resembla {

Feature::text_type DatePeriodFeatureExtractor::operator()(const string_type&) const
{
   auto t = std::time(nullptr);
   auto* lt = std::localtime(&t);
   std::stringstream ss;
   ss.imbue(std::locale("C"));
   ss << std::setw(4) << std::setfill('0') << ((lt->tm_mon + 1) * 100 + lt->tm_mday);
   return ss.str();
}

}
