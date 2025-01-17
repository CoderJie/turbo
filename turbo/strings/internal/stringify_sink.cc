// Copyright 2022 The Turbo Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "turbo/strings/internal/stringify_sink.h"
namespace turbo {
TURBO_NAMESPACE_BEGIN
namespace strings_internal {

void StringifySink::Append(size_t count, char ch) { buffer_.append(count, ch); }

void StringifySink::Append(string_view v) {
  buffer_.append(v.data(), v.size());
}

}  // namespace strings_internal
TURBO_NAMESPACE_END
}  // namespace turbo
