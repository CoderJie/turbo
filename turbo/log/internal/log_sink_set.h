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
// -----------------------------------------------------------------------------
// File: log/internal/log_sink_set.h
// -----------------------------------------------------------------------------

#ifndef TURBO_LOG_INTERNAL_LOG_SINK_SET_H_
#define TURBO_LOG_INTERNAL_LOG_SINK_SET_H_

#include "turbo/log/log_entry.h"
#include "turbo/log/log_sink.h"
#include "turbo/meta/span.h"
#include "turbo/platform/port.h"

namespace turbo {
TURBO_NAMESPACE_BEGIN
namespace log_internal {

// Returns true if a globally-registered `LogSink`'s `Send()` is currently
// being invoked on this thread.
bool ThreadIsLoggingToLogSink();

// This function may log to two sets of sinks:
//
// * If `extra_sinks_only` is true, it will dispatch only to `extra_sinks`.
//   `LogMessage::ToSinkAlso` and `LogMessage::ToSinkOnly` are used to attach
//    extra sinks to the entry.
// * Otherwise it will also log to the global sinks set. This set is managed
//   by `turbo::AddLogSink` and `turbo::RemoveLogSink`.
void LogToSinks(const turbo::LogEntry& entry,
                turbo::Span<turbo::LogSink*> extra_sinks, bool extra_sinks_only);

// Implementation for operations with log sink set.
void AddLogSink(turbo::LogSink* sink);
void RemoveLogSink(turbo::LogSink* sink);
void FlushLogSinks();

}  // namespace log_internal
TURBO_NAMESPACE_END
}  // namespace turbo

#endif  // TURBO_LOG_INTERNAL_LOG_SINK_SET_H_
