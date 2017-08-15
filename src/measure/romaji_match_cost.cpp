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

#include <fstream>
#include <algorithm>

#include "romaji_match_cost.hpp"

namespace resembla {

const std::unordered_set<string_type> RomajiMatchCost::DEFAULT_SIMILAR_LETTER_PAIRS = {
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
    case_mismatch_cost(case_mismatch_cost), similar_letter_cost(similar_letter_cost)
{
    for(const auto& p: DEFAULT_SIMILAR_LETTER_PAIRS){
        letter_similarities[p] = similar_letter_cost;
    }
}

RomajiMatchCost::RomajiMatchCost(const std::string& letter_similarity_file_path, double case_mismatch_cost, double similar_letter_cost):
    case_mismatch_cost(case_mismatch_cost), similar_letter_cost(similar_letter_cost)
{
    std::basic_ifstream<string_type::value_type> ifs(letter_similarity_file_path);
    if(ifs.fail()){
        throw std::runtime_error("input file is not available: " + letter_similarity_file_path);
    }
    auto delimiter = cast_string<string_type>(std::string("\t"));
    while(ifs.good()){
        string_type line;
        std::getline(ifs, line);
        if(ifs.eof() || line.length() == 0){
            break;
        }

        auto columns = split(line, L'\t');
        if(columns.size() < 2){
            throw std::runtime_error("invalid line in " + letter_similarity_file_path + ": " + cast_string<std::string>(line));
        }
        auto letters= columns[0];
        auto cost = std::stod(columns[1]);

        std::sort(std::begin(letters), std::end(letters));
        for(size_t i = 0; i < letters.size(); ++i){
            string_type p(1, letters[i]);
            for(size_t j = i + 1; i < letters.size(); ++i){
                letter_similarities[p + letters[j]] = cost;
            }
        }
    }
}

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
        auto p = letter_similarities.find(string_type({al, bl}));
        if(p != std::end(letter_similarities)){
            if((a == al && b == bl) || (a != al && b != bl)){
                result = p->second;
            }
            else{
                // special case that cases are different but letters are similar
                result = std::min(result, case_mismatch_cost + p->second);
            }
        }
    }

    return result;
}

}
