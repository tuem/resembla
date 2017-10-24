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

#include "mecab_util.hpp"

#include <fstream>
#include <stdexcept>

#include "string_util.hpp"

namespace resembla {

std::string validate_mecab_options(const std::string& mecab_options)
{
    bool dict_option_found = false;
    for(const auto& token: split(mecab_options, ' ')){
        if(token == " "){
            continue;
        }

        if(dict_option_found){
            std::ifstream ifs(token);
            if(ifs.fail()){
                throw std::runtime_error("MeCab dictionary is not available: " + token);
            }
            dict_option_found = false;
            break;
        }
        else if(token == "-d"){
            dict_option_found = true;
        }
    }
    if(dict_option_found){
        throw std::runtime_error("-d option is used without dictionary path");
    }
    return mecab_options;
}

}
