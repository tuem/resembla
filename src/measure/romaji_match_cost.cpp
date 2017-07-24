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

#include <algorithm>

#include "romaji_match_cost.hpp"

namespace resembla {

const std::unordered_set<string_type> RomajiMatchCost::SIMILAR_LETTER_PAIRS = {
	L"bv",
	L"ck",
	L"cq",
	L"kq",
	L"cs",
	L"fh",
	L"lr",
	L"jz",
	L"xz",
	L"-a",
	L"-i",
	L"-u",
	L"-e",
	L"-o",
};

RomajiMatchCost::RomajiMatchCost(double case_mismatch_cost, double similar_letter_cost): 
    case_mismatch_cost(case_mismatch_cost), similar_letter_cost(similar_letter_cost) {}

RomajiMatchCost::value_type RomajiMatchCost::toLower(value_type a) const
{
    if(L'A' <= a && a <= L'Z'){
        return a + (L'a' - L'A');
    }
    else{
        return a;
    }
}

double RomajiMatchCost::operator()(const value_type a, const value_type b) const
{
    if(a == b){
        return 0L;
    }

    double result = 1L;

    auto al = toLower(a), bl = toLower(b);
    if(al == bl){
        result = case_mismatch_cost;
    }
    else{
        if(bl < al){
            auto tmp = al;
            al = bl;
            bl = tmp;
        }
        if(SIMILAR_LETTER_PAIRS.count(string_type({al, bl})) > 0){
            if((a == al && b == bl) || (a != al && b != bl)){
                result = similar_letter_cost;
            }
            else{
                // special case that cases are different but letters are similar
                result = std::min(result, case_mismatch_cost + similar_letter_cost);
            }
        }
    }

    return result;
}

}
