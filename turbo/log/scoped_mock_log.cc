//
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

#include "turbo/log/scoped_mock_log.h"

#include <atomic>
#include <string>

#include "turbo/base/internal/raw_logging.h"
#include "turbo/log/log_entry.h"
#include "turbo/log/log_sink.h"
#include "turbo/log/log_sink_registry.h"
#include "turbo/platform/port.h"
#include "turbo/strings/string_view.h"
#include "gmock/gmock.h"

namespace turbo {
TURBO_NAMESPACE_BEGIN

ScopedMockLog::ScopedMockLog(MockLogDefault default_exp)
    : sink_(this), is_capturing_logs_(false) {
  if (default_exp == MockLogDefault::kIgnoreUnexpected) {
    // Ignore all calls to Log we did not set expectations for.
    EXPECT_CALL(*this, Log).Times(::testing::AnyNumber());
  } else {
    // Disallow all calls to Log we did not set expectations for.
    EXPECT_CALL(*this, Log).Times(0);
  }
  // By default Send mock forwards to Log mock.
  EXPECT_CALL(*this, Send)
      .Times(::testing::AnyNumber())
      .WillRepeatedly([this](const turbo::LogEntry& entry) {
        is_triggered_.store(true, std::memory_order_relaxed);
        Log(entry.log_severity(), std::string(entry.source_filename()),
            std::string(entry.text_message()));
      });

  // By default We ignore all Flush calls.
  EXPECT_CALL(*this, Flush).Times(::testing::AnyNumber());
}

ScopedMockLog::~ScopedMockLog() {
  TURBO_RAW_CHECK(is_triggered_.load(std::memory_order_relaxed),
                 "Did you forget to call StartCapturingLogs()?");

  if (is_capturing_logs_) StopCapturingLogs();
}

void ScopedMockLog::StartCapturingLogs() {
  TURBO_RAW_CHECK(!is_capturing_logs_,
                 "StartCapturingLogs() can be called only when the "
                 "turbo::ScopedMockLog object is not capturing logs.");

  is_capturing_logs_ = true;
  is_triggered_.store(true, std::memory_order_relaxed);
  turbo::AddLogSink(&sink_);
}

void ScopedMockLog::StopCapturingLogs() {
  TURBO_RAW_CHECK(is_capturing_logs_,
                 "StopCapturingLogs() can be called only when the "
                 "turbo::ScopedMockLog object is capturing logs.");

  is_capturing_logs_ = false;
  turbo::RemoveLogSink(&sink_);
}

turbo::LogSink& ScopedMockLog::UseAsLocalSink() {
  is_triggered_.store(true, std::memory_order_relaxed);
  return sink_;
}

TURBO_NAMESPACE_END
}  // namespace turbo
