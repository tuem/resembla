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

#include "word_mismatch_cost.hpp"

#include <algorithm>
#include <iterator>

namespace resembla {

double WordMismatchCost::operator()(const Word& reference, const Word& target) const
{
    if(reference.surface == target.surface){
        return 0.0;
    }
    else if((!reference.feature[6].empty() && reference.feature[6] != L"*" && reference.feature[6] == target.feature[6]) ||
            (!reference.feature[7].empty() && reference.feature[7] != L"*" && reference.feature[7] == target.feature[7])){
        return 0.1;
    }
    else{
        // compute symbol-based distance
        auto a = reference.surface, b = target.surface;
        sort(std::begin(a), std::end(a));
        sort(std::begin(b), std::end(b));
        size_t total = a.length() + b.length(), i = 0, j = 0, c = total;
        while(i < a.length() && j < b.length()){
            if(a[i] == b[j]){
                ++i;
                ++j;
                c -= 2;
            }
            else if(a[i] < b[j]){
                ++i;
            }
            else{
                ++j;
            }
        }
        return c / static_cast<double>(total);
    }
}

}
