// Copyright 2018 The Turbo Authors.
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
// -----------------------------------------------------------------------------
// mocking_bit_gen.h
// -----------------------------------------------------------------------------
//
// This file includes an `turbo::MockingBitGen` class to use as a mock within the
// Googletest testing framework. Such a mock is useful to provide deterministic
// values as return values within (otherwise random) Turbo distribution
// functions. Such determinism within a mock is useful within testing frameworks
// to test otherwise indeterminate APIs.
//
// More information about the Googletest testing framework is available at
// https://github.com/google/googletest

#ifndef TURBO_RANDOM_MOCKING_BIT_GEN_H_
#define TURBO_RANDOM_MOCKING_BIT_GEN_H_

#include <iterator>
#include <limits>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "turbo/base/internal/fast_type_id.h"
#include "turbo/container/flat_hash_map.h"
#include "turbo/meta/type_traits.h"
#include "turbo/random/distributions.h"
#include "turbo/random/internal/distribution_caller.h"
#include "turbo/random/random.h"
#include "turbo/strings/str_cat.h"
#include "turbo/strings/str_join.h"
#include "turbo/meta/span.h"
#include "turbo/meta/variant.h"
#include "turbo/meta/utility.h"

namespace turbo {
TURBO_NAMESPACE_BEGIN

namespace random_internal {
template <typename>
struct DistributionCaller;
class MockHelpers;

}  // namespace random_internal
class BitGenRef;

// MockingBitGen
//
// `turbo::MockingBitGen` is a mock Uniform Random Bit Generator (URBG) class
// which can act in place of an `turbo::BitGen` URBG within tests using the
// Googletest testing framework.
//
// Usage:
//
// Use an `turbo::MockingBitGen` along with a mock distribution object (within
// mock_distributions.h) inside Googletest constructs such as ON_CALL(),
// EXPECT_TRUE(), etc. to produce deterministic results conforming to the
// distribution's API contract.
//
// Example:
//
//  // Mock a call to an `turbo::Bernoulli` distribution using Googletest
//   turbo::MockingBitGen bitgen;
//
//   ON_CALL(turbo::MockBernoulli(), Call(bitgen, 0.5))
//       .WillByDefault(testing::Return(true));
//   EXPECT_TRUE(turbo::Bernoulli(bitgen, 0.5));
//
//  // Mock a call to an `turbo::Uniform` distribution within Googletest
//  turbo::MockingBitGen bitgen;
//
//   ON_CALL(turbo::MockUniform<int>(), Call(bitgen, testing::_, testing::_))
//       .WillByDefault([] (int low, int high) {
//           return low + (high - low) / 2;
//       });
//
//   EXPECT_EQ(turbo::Uniform<int>(gen, 0, 10), 5);
//   EXPECT_EQ(turbo::Uniform<int>(gen, 30, 40), 35);
//
// At this time, only mock distributions supplied within the Turbo random
// library are officially supported.
//
// EXPECT_CALL and ON_CALL need to be made within the same DLL component as
// the call to turbo::Uniform and related methods, otherwise mocking will fail
// since the  underlying implementation creates a type-specific pointer which
// will be distinct across different DLL boundaries.
//
class MockingBitGen {
 public:
  MockingBitGen() = default;
  ~MockingBitGen() = default;

  // URBG interface
  using result_type = turbo::BitGen::result_type;

  static constexpr result_type(min)() { return (turbo::BitGen::min)(); }
  static constexpr result_type(max)() { return (turbo::BitGen::max)(); }
  result_type operator()() { return gen_(); }

 private:
  // GetMockFnType returns the testing::MockFunction for a result and tuple.
  // This method only exists for type deduction and is otherwise unimplemented.
  template <typename ResultT, typename... Args>
  static auto GetMockFnType(ResultT, std::tuple<Args...>)
      -> ::testing::MockFunction<ResultT(Args...)>;

  // MockFnCaller is a helper method for use with turbo::apply to
  // apply an ArgTupleT to a compatible MockFunction.
  // NOTE: MockFnCaller is essentially equivalent to the lambda:
  // [fn](auto... args) { return fn->Call(std::move(args)...)}
  // however that fails to build on some supported platforms.
  template <typename MockFnType, typename ResultT, typename Tuple>
  struct MockFnCaller;

  // specialization for std::tuple.
  template <typename MockFnType, typename ResultT, typename... Args>
  struct MockFnCaller<MockFnType, ResultT, std::tuple<Args...>> {
    MockFnType* fn;
    inline ResultT operator()(Args... args) {
      return fn->Call(std::move(args)...);
    }
  };

  // FunctionHolder owns a particular ::testing::MockFunction associated with
  // a mocked type signature, and implement the type-erased Apply call, which
  // applies type-erased arguments to the mock.
  class FunctionHolder {
   public:
    virtual ~FunctionHolder() = default;

    // Call is a dispatch function which converts the
    // generic type-erased parameters into a specific mock invocation call.
    virtual void Apply(/*ArgTupleT*/ void* args_tuple,
                       /*ResultT*/ void* result) = 0;
  };

  template <typename MockFnType, typename ResultT, typename ArgTupleT>
  class FunctionHolderImpl final : public FunctionHolder {
   public:
    void Apply(void* args_tuple, void* result) override {
      // Requires tuple_args to point to a ArgTupleT, which is a
      // std::tuple<Args...> used to invoke the mock function. Requires result
      // to point to a ResultT, which is the result of the call.
      *static_cast<ResultT*>(result) =
          turbo::apply(MockFnCaller<MockFnType, ResultT, ArgTupleT>{&mock_fn_},
                      *static_cast<ArgTupleT*>(args_tuple));
    }

    MockFnType mock_fn_;
  };

  // MockingBitGen::RegisterMock
  //
  // RegisterMock<ResultT, ArgTupleT>(FastTypeIdType) is the main extension
  // point for extending the MockingBitGen framework. It provides a mechanism to
  // install a mock expectation for a function like ResultT(Args...) keyed by
  // type_idex onto the MockingBitGen context. The key is that the type_index
  // used to register must match the type index used to call the mock.
  //
  // The returned MockFunction<...> type can be used to setup additional
  // distribution parameters of the expectation.
  template <typename ResultT, typename ArgTupleT, typename SelfT>
  auto RegisterMock(SelfT&, base_internal::FastTypeIdType type)
      -> decltype(GetMockFnType(std::declval<ResultT>(),
                                std::declval<ArgTupleT>()))& {
    using MockFnType = decltype(GetMockFnType(std::declval<ResultT>(),
                                              std::declval<ArgTupleT>()));

    using WrappedFnType = turbo::conditional_t<
        std::is_same<SelfT, ::testing::NiceMock<turbo::MockingBitGen>>::value,
        ::testing::NiceMock<MockFnType>,
        turbo::conditional_t<
            std::is_same<SelfT,
                         ::testing::NaggyMock<turbo::MockingBitGen>>::value,
            ::testing::NaggyMock<MockFnType>,
            turbo::conditional_t<
                std::is_same<SelfT,
                             ::testing::StrictMock<turbo::MockingBitGen>>::value,
                ::testing::StrictMock<MockFnType>, MockFnType>>>;

    using ImplT = FunctionHolderImpl<WrappedFnType, ResultT, ArgTupleT>;
    auto& mock = mocks_[type];
    if (!mock) {
      mock = turbo::make_unique<ImplT>();
    }
    return static_cast<ImplT*>(mock.get())->mock_fn_;
  }

  // MockingBitGen::InvokeMock
  //
  // InvokeMock(FastTypeIdType, args, result) is the entrypoint for invoking
  // mocks registered on MockingBitGen.
  //
  // When no mocks are registered on the provided FastTypeIdType, returns false.
  // Otherwise attempts to invoke the mock function ResultT(Args...) that
  // was previously registered via the type_index.
  // Requires tuple_args to point to a ArgTupleT, which is a std::tuple<Args...>
  // used to invoke the mock function.
  // Requires result to point to a ResultT, which is the result of the call.
  inline bool InvokeMock(base_internal::FastTypeIdType type, void* args_tuple,
                         void* result) {
    // Trigger a mock, if there exists one that matches `param`.
    auto it = mocks_.find(type);
    if (it == mocks_.end()) return false;
    it->second->Apply(args_tuple, result);
    return true;
  }

  turbo::flat_hash_map<base_internal::FastTypeIdType,
                      std::unique_ptr<FunctionHolder>>
      mocks_;
  turbo::BitGen gen_;

  template <typename>
  friend struct ::turbo::random_internal::DistributionCaller;  // for InvokeMock
  friend class ::turbo::BitGenRef;                             // for InvokeMock
  friend class ::turbo::random_internal::MockHelpers;  // for RegisterMock,
                                                      // InvokeMock
};

TURBO_NAMESPACE_END
}  // namespace turbo

#endif  // TURBO_RANDOM_MOCKING_BIT_GEN_H_
