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

#include "turbo/time/internal/test_util.h"

#include "turbo/base/internal/raw_logging.h"
#include "turbo/platform/port.h"

namespace turbo {
TURBO_NAMESPACE_BEGIN
namespace time_internal {

TimeZone LoadTimeZone(const std::string& name) {
  TimeZone tz;
  TURBO_RAW_CHECK(LoadTimeZone(name, &tz), name.c_str());
  return tz;
}

}  // namespace time_internal
TURBO_NAMESPACE_END
}  // namespace turbo
