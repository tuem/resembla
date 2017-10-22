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

#ifndef RESEMBLA_COMPOSITION_HPP
#define RESEMBLA_COMPOSITION_HPP

#include <memory>

namespace resembla {

template<class F, class G>
class Composition
{
public:
    using input_type = typename F::input_type;
    using output_type = typename G::output_type;

    Composition(const std::shared_ptr<F> f, const std::shared_ptr<G> g):
        f(f), g(g)
    {}

    output_type operator()(const input_type& a, const input_type& b) const
    {
        return (*g)((*f)(a, b));
    }

protected:
    const std::shared_ptr<F> f;
    const std::shared_ptr<G> g;
};

}
#endif
