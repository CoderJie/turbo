//
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

#include "turbo/flags/internal/private_handle_accessor.h"

#include <memory>
#include <string>

#include "turbo/flags/commandlineflag.h"
#include "turbo/flags/internal/commandlineflag.h"
#include "turbo/platform/port.h"
#include "turbo/strings/string_view.h"

namespace turbo {
TURBO_NAMESPACE_BEGIN
namespace flags_internal {

FlagFastTypeId PrivateHandleAccessor::TypeId(const CommandLineFlag& flag) {
  return flag.TypeId();
}

std::unique_ptr<FlagStateInterface> PrivateHandleAccessor::SaveState(
    CommandLineFlag& flag) {
  return flag.SaveState();
}

bool PrivateHandleAccessor::IsSpecifiedOnCommandLine(
    const CommandLineFlag& flag) {
  return flag.IsSpecifiedOnCommandLine();
}

bool PrivateHandleAccessor::ValidateInputValue(const CommandLineFlag& flag,
                                               turbo::string_view value) {
  return flag.ValidateInputValue(value);
}

void PrivateHandleAccessor::CheckDefaultValueParsingRoundtrip(
    const CommandLineFlag& flag) {
  flag.CheckDefaultValueParsingRoundtrip();
}

bool PrivateHandleAccessor::ParseFrom(CommandLineFlag& flag,
                                      turbo::string_view value,
                                      flags_internal::FlagSettingMode set_mode,
                                      flags_internal::ValueSource source,
                                      std::string& error) {
  return flag.ParseFrom(value, set_mode, source, error);
}

}  // namespace flags_internal
TURBO_NAMESPACE_END
}  // namespace turbo

