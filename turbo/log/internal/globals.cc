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

#include "turbo/log/internal/globals.h"

#include <atomic>
#include <cstdio>

#include "turbo/base/internal/raw_logging.h"
#include "turbo/base/log_severity.h"
#include "turbo/platform/port.h"
#include "turbo/strings/string_view.h"
#include "turbo/time/time.h"

namespace turbo {
TURBO_NAMESPACE_BEGIN
namespace log_internal {

namespace {
// Keeps track of whether Logging initialization is finalized.
// Log messages generated before that will go to stderr.
TURBO_CONST_INIT std::atomic<bool> logging_initialized(false);

// The TimeZone used for logging. This may only be set once.
TURBO_CONST_INIT std::atomic<turbo::TimeZone*> timezone_ptr{nullptr};

// If true, the logging library will symbolize stack in fatal messages
TURBO_CONST_INIT std::atomic<bool> symbolize_stack_trace(true);

// Specifies maximum number of stack frames to report in fatal messages.
TURBO_CONST_INIT std::atomic<int> max_frames_in_stack_trace(64);

TURBO_CONST_INIT std::atomic<bool> exit_on_dfatal(true);
TURBO_CONST_INIT std::atomic<bool> suppress_sigabort_trace(false);
}  // namespace

bool IsInitialized() {
  return logging_initialized.load(std::memory_order_acquire);
}

void SetInitialized() {
  logging_initialized.store(true, std::memory_order_release);
}

void WriteToStderr(turbo::string_view message, turbo::LogSeverity severity) {
  // Avoid using std::cerr from this module since we may get called during
  // exit code, and cerr may be partially or fully destroyed by then.
  std::fwrite(message.data(), message.size(), 1, stderr);

#if defined(_WIN64) || defined(_WIN32) || defined(_WIN16)
  // C99 requires stderr to not be fully-buffered by default (7.19.3.7), but
  // MS CRT buffers it anyway, so we must `fflush` to ensure the string hits
  // the console/file before the program dies (and takes the libc buffers
  // with it).
  // https://docs.microsoft.com/en-us/cpp/c-runtime-library/stream-i-o
  if (severity >= turbo::LogSeverity::kWarning) {
    std::fflush(stderr);
  }
#else
  // Avoid unused parameter warning in this branch.
  (void)severity;
#endif
}

void SetTimeZone(turbo::TimeZone tz) {
  turbo::TimeZone* expected = nullptr;
  turbo::TimeZone* new_tz = new turbo::TimeZone(tz);
  // timezone_ptr can only be set once, otherwise new_tz is leaked.
  if (!timezone_ptr.compare_exchange_strong(expected, new_tz,
                                            std::memory_order_release,
                                            std::memory_order_relaxed)) {
    TURBO_RAW_LOG(FATAL,
                 "turbo::log_internal::SetTimeZone() has already been called");
  }
}

const turbo::TimeZone* TimeZone() {
  return timezone_ptr.load(std::memory_order_acquire);
}

bool ShouldSymbolizeLogStackTrace() {
  return symbolize_stack_trace.load(std::memory_order_acquire);
}

void EnableSymbolizeLogStackTrace(bool on_off) {
  symbolize_stack_trace.store(on_off, std::memory_order_release);
}

int MaxFramesInLogStackTrace() {
  return max_frames_in_stack_trace.load(std::memory_order_acquire);
}

void SetMaxFramesInLogStackTrace(int max_num_frames) {
  max_frames_in_stack_trace.store(max_num_frames, std::memory_order_release);
}

bool ExitOnDFatal() { return exit_on_dfatal.load(std::memory_order_acquire); }

void SetExitOnDFatal(bool on_off) {
  exit_on_dfatal.store(on_off, std::memory_order_release);
}

bool SuppressSigabortTrace() {
  return suppress_sigabort_trace.load(std::memory_order_acquire);
}

bool SetSuppressSigabortTrace(bool on_off) {
  return suppress_sigabort_trace.exchange(on_off);
}

}  // namespace log_internal
TURBO_NAMESPACE_END
}  // namespace turbo
