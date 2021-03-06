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

#include <doctest/doctest.h>
#include <execution>

#include "core/common.hpp"
#include "core/dataset.hpp"
#include "core/eval.hpp"
#include "hash/hash.hpp"
#include "core/pset.hpp"
#include "operators/creator.hpp"
#include "analyzers/diversity.hpp"

#include "nanobench.h"

namespace Operon {
namespace Test {

TEST_CASE("Hashing performance") {
    size_t nTrees = 1000;
    size_t maxLength = 200;
    size_t maxDepth = 1000;

    Operon::RandomGenerator rd(std::random_device{}());
    auto ds = Dataset("../data/Poly-10.csv", true);

    auto target = "Y";
    auto variables = ds.Variables();
    std::vector<Variable> inputs;
    std::copy_if(variables.begin(), variables.end(), std::back_inserter(inputs), [&](const auto& v) { return v.Name != target; });

    std::uniform_int_distribution<size_t> sizeDistribution(1, maxLength);
    std::uniform_int_distribution<size_t> dist(0, nTrees-1);

    PrimitiveSet grammar;
    grammar.SetConfig(PrimitiveSet::Arithmetic);

    std::vector<Tree> trees(nTrees);
    auto btc = BalancedTreeCreator { grammar, inputs };

    ankerl::nanobench::Bench b;
    b.relative(true).performanceCounters(true).minEpochIterations(10);
    std::generate(std::execution::seq, trees.begin(), trees.end(), [&]() { return btc(rd, sizeDistribution(rd), 0, maxDepth); });

    const auto countTotalNodes = [&]() { return std::transform_reduce(std::execution::par_unseq, trees.begin(), trees.end(), size_t { 0 }, std::plus<size_t> {}, [](auto& tree) { return tree.Length(); }); };

    auto totalNodes = countTotalNodes();

    std::vector<std::pair<Operon::HashFunction, std::string>> hashFunctions {
        { Operon::HashFunction::XXHash,    "XXHash" },
        { Operon::HashFunction::MetroHash, "MetroHash" },
        { Operon::HashFunction::FNV1Hash,  "FNV1Hash" },
    };

    SUBCASE("strict hashing") {
        for (const auto& [f, n] : hashFunctions) {
            b.batch(totalNodes).run(n, [&, f=f]() { 
                std::for_each(trees.begin(), trees.end(), [&](Tree& t) { t.Hash(f, Operon::HashMode::Strict); });
            });
        }
    }

    SUBCASE("strict hashing + sort") {
        for (const auto& [f, n] : hashFunctions) {
            b.batch(totalNodes).run(n, [&, f=f]() { 
                std::for_each(trees.begin(), trees.end(), [&](Tree& t) { t.Hash(f, Operon::HashMode::Strict).Sort(); });
            });
        }
    }

    SUBCASE("struct hashing") {
        for (const auto& [f, n] : hashFunctions) {
            b.batch(totalNodes).run(n, [&, f=f]() { 
                std::for_each(trees.begin(), trees.end(), [&](Tree& t) { t.Hash(f, Operon::HashMode::Relaxed); });
            });
        }
    }

    SUBCASE("struct hashing + sort") {
        for (const auto& [f, n] : hashFunctions) {
            b.batch(totalNodes).run(n, [&, f=f]() { 
                std::for_each(trees.begin(), trees.end(), [&](Tree& t) { t.Hash(f, Operon::HashMode::Relaxed).Sort(); });
            });
        }
    }

    SUBCASE("strict hashing complexity") {
        for (size_t i = 1; i <= maxLength; ++i) {
            std::generate(std::execution::seq, trees.begin(), trees.end(), [&]() { return btc(rd, i, 0, maxDepth); });
            totalNodes = countTotalNodes();
            b.complexityN(i).batch(totalNodes).run("strict", [&]() { 
                ankerl::nanobench::doNotOptimizeAway(std::for_each(trees.begin(), trees.end(), [](auto t) { t.Sort(); }));
            });
        }

        std::cout << b.complexityBigO() << "\n";
    }

    SUBCASE("relaxed hashing complexity") {
        for (size_t i = 1; i <= maxLength; ++i) {
            std::generate(std::execution::seq, trees.begin(), trees.end(), [&]() { return btc(rd, i, 0, maxDepth); });
            totalNodes = countTotalNodes();
            b.complexityN(i).batch(totalNodes).run("relaxed", [&]() { 
                ankerl::nanobench::doNotOptimizeAway(std::for_each(trees.begin(), trees.end(), [](auto t) { t.Sort(); }));
            });
        }

        std::cout << b.complexityBigO() << "\n";
    }
}

} // namespace Test 
} // namespace Operon

