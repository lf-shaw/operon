/* This file is part of:
 * Operon - Large Scale Genetic Programming Framework
 *
 * Licensed under the ISC License <https://opensource.org/licenses/ISC> 
 * Copyright (C) 2019 Bogdan Burlacu 
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE. 
 */

#ifndef OPERON_REINSERTER_REPLACE_WORST
#define OPERON_REINSERTER_REPLACE_WORST

#include "core/operator.hpp"
#include <cstddef>

namespace Operon {
template <typename ExecutionPolicy = std::execution::parallel_unsequenced_policy>
class ReplaceWorstReinserter : public ReinserterBase {
    public:
        explicit ReplaceWorstReinserter(ComparisonCallback&& cb) : ReinserterBase(cb) { }
        explicit ReplaceWorstReinserter(ComparisonCallback const& cb) : ReinserterBase(cb) { }
        // replace the worst individuals in pop with the best individuals from pool
        void operator()(Operon::RandomGenerator&, std::vector<Individual>& pop, std::vector<Individual>& pool) const override {
            // typically the pool and the population are the same size
            if (pop.size() == pool.size()) {
                pop.swap(pool);
                return;
            }

            ExecutionPolicy ep;
            if (pop.size() > pool.size()) {
                std::sort(ep, pop.begin(), pop.end(), this->comp);
            } else if (pop.size() < pool.size()) {
                std::sort(ep, pool.begin(), pool.end(), this->comp);
            }
            auto offset = static_cast<std::ptrdiff_t>(std::min(pop.size(), pool.size()));
            std::swap_ranges(pool.begin(), pool.begin() + offset, pop.end() - offset);
        }
};
} // namespace operon

#endif
