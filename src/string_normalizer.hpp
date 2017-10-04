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

#ifndef RESEMBLA_STRING_NORMALIZER_HPP
#define RESEMBLA_STRING_NORMALIZER_HPP

#include <string>
#include <memory>

#include <unicode/translit.h>

#include "symbol_normalizer.hpp"

namespace resembla {

class StringNormalizer final: public SymbolNormalizer
{
public:
    StringNormalizer(const std::string& nrm_dir, const std::string& nrm_name,
        const std::string& predefined_nrm_name, const std::string& transliteration_path,
        bool to_lower = false);

    template<typename string_type>
    string_type operator()(const string_type& input) const
    {
        if(normalizer_resembla == nullptr && normalizer_nfkc == nullptr &&
                transliterator == nullptr && !to_lower){
            return input;
        }

        UErrorCode error_code = U_ZERO_ERROR;
        auto work = cast_string<UnicodeString>(input);
        if(normalizer_resembla != nullptr){
            work = normalizer_resembla->normalize(work, error_code);
            if(U_FAILURE(error_code)){
                throw std::runtime_error("failed to normalize input");
            }
        }
        if(normalizer_nfkc != nullptr){
            work = normalizer_nfkc->normalize(work, error_code);
            if(U_FAILURE(error_code)){
                throw std::runtime_error("failed to normalize input");
            }
        }
        if(transliterator != nullptr){
            transliterator->transliterate(work);
        }

        return cast_string<string_type>(to_lower ? work.toLower() : work);
    }

protected:
    std::shared_ptr<icu::Transliterator> transliterator;
    bool to_lower;
};

}
#endif
