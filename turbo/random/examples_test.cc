// Copyright 2020 The Turbo Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <cinttypes>
#include <random>
#include <sstream>
#include <vector>

#include "gtest/gtest.h"
#include "turbo/random/random.h"

template <typename T>
void Use(T) {}

TEST(Examples, Basic) {
  turbo::BitGen gen;
  std::vector<int> objs = {10, 20, 30, 40, 50};

  // Choose an element from a set.
  auto elem = objs[turbo::Uniform(gen, 0u, objs.size())];
  Use(elem);

  // Generate a uniform value between 1 and 6.
  auto dice_roll = turbo::Uniform<int>(turbo::IntervalClosedClosed, gen, 1, 6);
  Use(dice_roll);

  // Generate a random byte.
  auto byte = turbo::Uniform<uint8_t>(gen);
  Use(byte);

  // Generate a fractional value from [0f, 1f).
  auto fraction = turbo::Uniform<float>(gen, 0, 1);
  Use(fraction);

  // Toss a fair coin; 50/50 probability.
  bool coin_toss = turbo::Bernoulli(gen, 0.5);
  Use(coin_toss);

  // Select a file size between 1k and 10MB, biased towards smaller file sizes.
  auto file_size = turbo::LogUniform<size_t>(gen, 1000, 10 * 1000 * 1000);
  Use(file_size);

  // Randomize (shuffle) a collection.
  std::shuffle(std::begin(objs), std::end(objs), gen);
}

TEST(Examples, CreateingCorrelatedVariateSequences) {
  // Unexpected PRNG correlation is often a source of bugs,
  // so when using turbo::BitGen it must be an intentional choice.
  // NOTE: All of these only exhibit process-level stability.

  // Create a correlated sequence from system entropy.
  {
    auto my_seed = turbo::MakeSeedSeq();

    turbo::BitGen gen_1(my_seed);
    turbo::BitGen gen_2(my_seed);  // Produces same variates as gen_1.

    EXPECT_EQ(turbo::Bernoulli(gen_1, 0.5), turbo::Bernoulli(gen_2, 0.5));
    EXPECT_EQ(turbo::Uniform<uint32_t>(gen_1), turbo::Uniform<uint32_t>(gen_2));
  }

  // Create a correlated sequence from an existing URBG.
  {
    turbo::BitGen gen;

    auto my_seed = turbo::CreateSeedSeqFrom(&gen);
    turbo::BitGen gen_1(my_seed);
    turbo::BitGen gen_2(my_seed);

    EXPECT_EQ(turbo::Bernoulli(gen_1, 0.5), turbo::Bernoulli(gen_2, 0.5));
    EXPECT_EQ(turbo::Uniform<uint32_t>(gen_1), turbo::Uniform<uint32_t>(gen_2));
  }

  // An alternate construction which uses user-supplied data
  // instead of a random seed.
  {
    const char kData[] = "A simple seed string";
    std::seed_seq my_seed(std::begin(kData), std::end(kData));

    turbo::BitGen gen_1(my_seed);
    turbo::BitGen gen_2(my_seed);

    EXPECT_EQ(turbo::Bernoulli(gen_1, 0.5), turbo::Bernoulli(gen_2, 0.5));
    EXPECT_EQ(turbo::Uniform<uint32_t>(gen_1), turbo::Uniform<uint32_t>(gen_2));
  }
}

