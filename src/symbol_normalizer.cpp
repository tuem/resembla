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

#include "symbol_normalizer.hpp"

#include <stdexcept>

#include <unicode/unistr.h>

namespace resembla {

SymbolNormalizer::SymbolNormalizer(const std::string& nrm_dir, const std::string& nrm_name,
        const std::string& predefined_nrm_name, bool to_lower): to_lower(to_lower)
{
    UErrorCode error_code = U_ZERO_ERROR;
    normalizer_resembla = !nrm_dir.empty() ?
        Normalizer2::getInstance(nrm_dir.c_str(), nrm_name.c_str(), UNORM2_COMPOSE, error_code) : nullptr;
    if(normalizer_resembla != nullptr && U_FAILURE(error_code)) {
        throw std::runtime_error("failed to initialize normalizer");
    }
    normalizer_nfkc = !predefined_nrm_name.empty() ?
        Normalizer2::getInstance(NULL, predefined_nrm_name.c_str(), UNORM2_COMPOSE, error_code) : nullptr;
    if(normalizer_nfkc != nullptr && U_FAILURE(error_code)) {
        throw std::runtime_error("failed to initialize normalizer");
    }
}

}
