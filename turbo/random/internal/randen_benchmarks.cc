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
//
#include "turbo/random/internal/randen.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

#include "turbo/base/internal/raw_logging.h"
#include "turbo/random/internal/nanobenchmark.h"
#include "turbo/random/internal/platform.h"
#include "turbo/random/internal/randen_engine.h"
#include "turbo/random/internal/randen_hwaes.h"
#include "turbo/random/internal/randen_slow.h"
#include "turbo/strings/numbers.h"

namespace {

using turbo::random_internal::Randen;
using turbo::random_internal::RandenHwAes;
using turbo::random_internal::RandenSlow;

using turbo::random_internal_nanobenchmark::FuncInput;
using turbo::random_internal_nanobenchmark::FuncOutput;
using turbo::random_internal_nanobenchmark::InvariantTicksPerSecond;
using turbo::random_internal_nanobenchmark::MeasureClosure;
using turbo::random_internal_nanobenchmark::Params;
using turbo::random_internal_nanobenchmark::PinThreadToCPU;
using turbo::random_internal_nanobenchmark::Result;

// Local state parameters.
static constexpr size_t kStateSizeT = Randen::kStateBytes / sizeof(uint64_t);
static constexpr size_t kSeedSizeT = Randen::kSeedBytes / sizeof(uint32_t);

// Randen implementation benchmarks.
template <typename T>
struct AbsorbFn : public T {
  mutable uint64_t state[kStateSizeT] = {};
  mutable uint32_t seed[kSeedSizeT] = {};

  static constexpr size_t bytes() { return sizeof(seed); }

  FuncOutput operator()(const FuncInput num_iters) const {
    for (size_t i = 0; i < num_iters; ++i) {
      this->Absorb(seed, state);
    }
    return state[0];
  }
};

template <typename T>
struct GenerateFn : public T {
  mutable uint64_t state[kStateSizeT];
  GenerateFn() { std::memset(state, 0, sizeof(state)); }

  static constexpr size_t bytes() { return sizeof(state); }

  FuncOutput operator()(const FuncInput num_iters) const {
    const auto* keys = this->GetKeys();
    for (size_t i = 0; i < num_iters; ++i) {
      this->Generate(keys, state);
    }
    return state[0];
  }
};

template <typename UInt>
struct Engine {
  mutable turbo::random_internal::randen_engine<UInt> rng;

  static constexpr size_t bytes() { return sizeof(UInt); }

  FuncOutput operator()(const FuncInput num_iters) const {
    for (size_t i = 0; i < num_iters - 1; ++i) {
      rng();
    }
    return rng();
  }
};

template <size_t N>
void Print(const char* name, const size_t n, const Result (&results)[N],
           const size_t bytes) {
  if (n == 0) {
    TURBO_RAW_LOG(
        WARNING,
        "WARNING: Measurement failed, should not happen when using "
        "PinThreadToCPU unless the region to measure takes > 1 second.\n");
    return;
  }

  static const double ns_per_tick = 1e9 / InvariantTicksPerSecond();
  static constexpr const double kNsPerS = 1e9;                 // ns/s
  static constexpr const double kMBPerByte = 1.0 / 1048576.0;  // Mb / b
  static auto header = [] {
    return printf("%20s %8s: %12s ticks; %9s  (%9s) %8s\n", "Name", "Count",
                  "Total", "Variance", "Time", "bytes/s");
  }();
  (void)header;

  for (size_t i = 0; i < n; ++i) {
    const double ticks_per_call = results[i].ticks / results[i].input;
    const double ns_per_call = ns_per_tick * ticks_per_call;
    const double bytes_per_ns = bytes / ns_per_call;
    const double mb_per_s = bytes_per_ns * kNsPerS * kMBPerByte;
    // Output
    printf("%20s %8zu: %12.2f ticks; MAD=%4.2f%%  (%6.1f ns) %8.1f Mb/s\n",
           name, results[i].input, results[i].ticks,
           results[i].variability * 100.0, ns_per_call, mb_per_s);
  }
}

// Fails here
template <typename Op, size_t N>
void Measure(const char* name, const FuncInput (&inputs)[N]) {
  Op op;

  Result results[N];
  Params params;
  params.verbose = false;
  params.max_evals = 6;  // avoid test timeout
  const size_t num_results = MeasureClosure(op, inputs, N, results, params);
  Print(name, num_results, results, op.bytes());
}

// unpredictable == 1 but the compiler does not know that.
void RunAll(const int argc, char* argv[]) {
  if (argc == 2) {
    int cpu = -1;
    if (!turbo::SimpleAtoi(argv[1], &cpu)) {
      TURBO_RAW_LOG(FATAL, "The optional argument must be a CPU number >= 0.\n");
    }
    PinThreadToCPU(cpu);
  }

  // The compiler cannot reduce this to a constant.
  const FuncInput unpredictable = (argc != 999);
  static const FuncInput inputs[] = {unpredictable * 100, unpredictable * 1000};

#if !defined(TURBO_INTERNAL_DISABLE_AES) && TURBO_HAVE_ACCELERATED_AES
  Measure<AbsorbFn<RandenHwAes>>("Absorb (HwAes)", inputs);
#endif
  Measure<AbsorbFn<RandenSlow>>("Absorb (Slow)", inputs);

#if !defined(TURBO_INTERNAL_DISABLE_AES) && TURBO_HAVE_ACCELERATED_AES
  Measure<GenerateFn<RandenHwAes>>("Generate (HwAes)", inputs);
#endif
  Measure<GenerateFn<RandenSlow>>("Generate (Slow)", inputs);

  // Measure the production engine.
  static const FuncInput inputs1[] = {unpredictable * 1000,
                                      unpredictable * 10000};
  Measure<Engine<uint64_t>>("randen_engine<uint64_t>", inputs1);
  Measure<Engine<uint32_t>>("randen_engine<uint32_t>", inputs1);
}

}  // namespace

int main(int argc, char* argv[]) {
  RunAll(argc, argv);
  return 0;
}
