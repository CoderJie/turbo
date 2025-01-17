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

#ifndef TURBO_TIME_INTERNAL_TEST_UTIL_H_
#define TURBO_TIME_INTERNAL_TEST_UTIL_H_

#include <string>

#include "turbo/time/time.h"

namespace turbo {
TURBO_NAMESPACE_BEGIN
namespace time_internal {

// Loads the named timezone, but dies on any failure.
turbo::TimeZone LoadTimeZone(const std::string& name);

}  // namespace time_internal
TURBO_NAMESPACE_END
}  // namespace turbo

#endif  // TURBO_TIME_INTERNAL_TEST_UTIL_H_
