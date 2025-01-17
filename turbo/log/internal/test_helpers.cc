// Copyright 2022 The Turbo Authors.
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
#include "turbo/log/internal/test_helpers.h"

#ifdef __Fuchsia__
#include <zircon/syscalls.h>
#endif

#include "turbo/base/log_severity.h"
#include "turbo/log/globals.h"
#include "turbo/log/initialize.h"
#include "turbo/log/internal/globals.h"
#include "turbo/platform/port.h"
#include "gtest/gtest.h"

namespace turbo {
TURBO_NAMESPACE_BEGIN
namespace log_internal {

// Returns false if the specified severity level is disabled by
// `TURBO_MIN_LOG_LEVEL` or `turbo::MinLogLevel()`.
bool LoggingEnabledAt(turbo::LogSeverity severity) {
  return severity >= kTurboMinLogLevel && severity >= turbo::MinLogLevel();
}

// -----------------------------------------------------------------------------
// Googletest Death Test Predicates
// -----------------------------------------------------------------------------

#if GTEST_HAS_DEATH_TEST

bool DiedOfFatal(int exit_status) {
#if defined(_WIN32)
  // Depending on NDEBUG and (configuration?) MSVC's abort either results
  // in error code 3 (SIGABRT) or error code 0x80000003 (breakpoint
  // triggered).
  return ::testing::ExitedWithCode(3)(exit_status & 0x7fffffff);
#elif defined(__Fuchsia__)
  // The Fuchsia death test implementation kill()'s the process when it detects
  // an exception, so it should exit with the corresponding code. See
  // FuchsiaDeathTest::Wait().
  return ::testing::ExitedWithCode(ZX_TASK_RETCODE_SYSCALL_KILL)(exit_status);
#elif defined(__ANDROID__) && defined(__aarch64__)
  // These are all run under a qemu config that eats died-due-to-signal exit
  // statuses.
  return true;
#else
  return ::testing::KilledBySignal(SIGABRT)(exit_status);
#endif
}

bool DiedOfQFatal(int exit_status) {
  return ::testing::ExitedWithCode(1)(exit_status);
}

#endif

// -----------------------------------------------------------------------------
// Helper for Log inititalization in test
// -----------------------------------------------------------------------------

void LogTestEnvironment::SetUp() {
  if (!turbo::log_internal::IsInitialized()) {
    turbo::InitializeLog();
  }
}

}  // namespace log_internal
TURBO_NAMESPACE_END
}  // namespace turbo
