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

#include <unicode/unistr.h>

#include "string_normalizer.hpp"
#include "string_util.hpp"

namespace resembla {

StringNormalizer::StringNormalizer(const std::string& nrm_dir, const std::string& nrm_name,
        const std::string& predefined_nrm_name, const std::string& transliteration_path,
        bool to_lower):
    SymbolNormalizer(nrm_dir, nrm_name, predefined_nrm_name),
    transliterator(nullptr), to_lower(to_lower)
{
    std::string rules;
    std::ifstream ifs(transliteration_path);
    if(!ifs.is_open()){
        return;
    }
    while(ifs.good()){
        std::string line;
        std::getline(ifs, line);
        if(ifs.eof()){
            break;
        }
        rules += line;
    }

    UParseError parse_error;
    UErrorCode error_code = U_ZERO_ERROR;
    transliterator.reset(Transliterator::createFromRules("resembla_transliteration", UnicodeString(rules.c_str()), UTRANS_FORWARD, parse_error, error_code));
}

StringNormalizer::~StringNormalizer(){}

bool StringNormalizer::available() const
{
    return SymbolNormalizer::available() && transliterator != nullptr;
}

string_type StringNormalizer::operator()(const string_type& input) const
{
    UErrorCode error_code = U_ZERO_ERROR;
    auto work = normalizer_resembla->normalize(cast_string<UnicodeString>(input), error_code);
    if(U_FAILURE(error_code)) {
        throw std::runtime_error("failed to normalize input");
    }
    work = normalizer_nfkc->normalize(work, error_code);
    if(U_FAILURE(error_code)) {
        throw std::runtime_error("failed to normalize input");
    }
    transliterator->transliterate(work);

    return cast_string<string_type>(to_lower ? work.toLower() : work);
}

}
