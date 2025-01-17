// Copyright 2023 The Turbo Authors.
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

#ifndef TURBO_BASE_ASSUME_H_
#define TURBO_BASE_ASSUME_H_

#include <cstdlib>
#include "turbo/platform/port.h"

namespace turbo {

TURBO_FORCE_INLINE void assume(bool cond) {
#if defined(__clang__) // Must go first because Clang also defines __GNUC__.
  __builtin_assume(cond);
#elif defined(__GNUC__)
  if (!cond) {
    __builtin_unreachable();
  }
#elif defined(_MSC_VER)
  __assume(cond);
#else
  // Do nothing.
#endif
}

TURBO_FORCE_INLINE void assume_unreachable() {
  assume(false);
  // Do a bit more to get the compiler to understand
  // that this function really will never return.
#if defined(__GNUC__)
  __builtin_unreachable();
#elif defined(_MSC_VER)
  __assume(0);
#else
  // Well, it's better than nothing.
  std::abort();
#endif
}

} // namespace turbo

#endif // TURBO_BASE_ASSUME_H_
