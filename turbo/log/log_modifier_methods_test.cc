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

#include <errno.h>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "turbo/log/internal/test_actions.h"
#include "turbo/log/internal/test_helpers.h"
#include "turbo/log/internal/test_matchers.h"
#include "turbo/log/log.h"
#include "turbo/log/log_sink.h"
#include "turbo/log/scoped_mock_log.h"
#include "turbo/strings/match.h"
#include "turbo/strings/string_view.h"
#include "turbo/time/time.h"

namespace {
#if GTEST_HAS_DEATH_TEST
using ::turbo::log_internal::DeathTestExpectedLogging;
using ::turbo::log_internal::DeathTestUnexpectedLogging;
using ::turbo::log_internal::DeathTestValidateExpectations;
using ::turbo::log_internal::DiedOfQFatal;
#endif
using ::turbo::log_internal::LogSeverity;
using ::turbo::log_internal::Prefix;
using ::turbo::log_internal::SourceBasename;
using ::turbo::log_internal::SourceFilename;
using ::turbo::log_internal::SourceLine;
using ::turbo::log_internal::Stacktrace;
using ::turbo::log_internal::TextMessage;
using ::turbo::log_internal::TextMessageWithPrefix;
using ::turbo::log_internal::TextMessageWithPrefixAndNewline;
using ::turbo::log_internal::TextPrefix;
using ::turbo::log_internal::ThreadID;
using ::turbo::log_internal::Timestamp;
using ::turbo::log_internal::Verbosity;

using ::testing::AllOf;
using ::testing::AnyNumber;
using ::testing::AnyOf;
using ::testing::Eq;
using ::testing::IsEmpty;
using ::testing::IsFalse;
using ::testing::Truly;

TEST(TailCallsModifiesTest, AtLocationFileLine) {
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(
      test_sink,
      Send(AllOf(
          // The metadata should change:
          SourceFilename(Eq("/my/very/very/very_long_source_file.cc")),
          SourceBasename(Eq("very_long_source_file.cc")), SourceLine(Eq(777)),
          // The logged line should change too, even though the prefix must
          // grow to fit the new metadata.
          TextMessageWithPrefix(Truly([](turbo::string_view msg) {
            return turbo::EndsWith(msg,
                                  " very_long_source_file.cc:777] hello world");
          })))));

  test_sink.StartCapturingLogs();
  LOG(INFO).AtLocation("/my/very/very/very_long_source_file.cc", 777)
      << "hello world";
}

TEST(TailCallsModifiesTest, NoPrefix) {
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(test_sink, Send(AllOf(Prefix(IsFalse()), TextPrefix(IsEmpty()),
                                    TextMessageWithPrefix(Eq("hello world")))));

  test_sink.StartCapturingLogs();
  LOG(INFO).NoPrefix() << "hello world";
}

TEST(TailCallsModifiesTest, NoPrefixNoMessageNoShirtNoShoesNoService) {
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(test_sink,
              Send(AllOf(Prefix(IsFalse()), TextPrefix(IsEmpty()),
                         TextMessageWithPrefix(IsEmpty()),
                         TextMessageWithPrefixAndNewline(Eq("\n")))));
  test_sink.StartCapturingLogs();
  LOG(INFO).NoPrefix();
}

TEST(TailCallsModifiesTest, WithVerbosity) {
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(test_sink, Send(Verbosity(Eq(2))));

  test_sink.StartCapturingLogs();
  LOG(INFO).WithVerbosity(2) << "hello world";
}

TEST(TailCallsModifiesTest, WithVerbosityNoVerbosity) {
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(test_sink,
              Send(Verbosity(Eq(turbo::LogEntry::kNoVerbosityLevel))));

  test_sink.StartCapturingLogs();
  LOG(INFO).WithVerbosity(2).WithVerbosity(turbo::LogEntry::kNoVerbosityLevel)
      << "hello world";
}

TEST(TailCallsModifiesTest, WithTimestamp) {
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(test_sink, Send(Timestamp(Eq(turbo::UnixEpoch()))));

  test_sink.StartCapturingLogs();
  LOG(INFO).WithTimestamp(turbo::UnixEpoch()) << "hello world";
}

TEST(TailCallsModifiesTest, WithThreadID) {
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(test_sink,
              Send(AllOf(ThreadID(Eq(turbo::LogEntry::tid_t{1234})))));

  test_sink.StartCapturingLogs();
  LOG(INFO).WithThreadID(1234) << "hello world";
}

TEST(TailCallsModifiesTest, WithMetadataFrom) {
  class ForwardingLogSink : public turbo::LogSink {
   public:
    void Send(const turbo::LogEntry &entry) override {
      LOG(LEVEL(entry.log_severity())).WithMetadataFrom(entry)
          << "forwarded: " << entry.text_message();
    }
  } forwarding_sink;

  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(
      test_sink,
      Send(AllOf(SourceFilename(Eq("fake/file")), SourceBasename(Eq("file")),
                 SourceLine(Eq(123)), Prefix(IsFalse()),
                 LogSeverity(Eq(turbo::LogSeverity::kWarning)),
                 Timestamp(Eq(turbo::UnixEpoch())),
                 ThreadID(Eq(turbo::LogEntry::tid_t{456})),
                 TextMessage(Eq("forwarded: hello world")), Verbosity(Eq(7)),
                 ENCODED_MESSAGE(
                     EqualsProto(R"pb(value { literal: "forwarded: " }
                                      value { str: "hello world" })pb")))));

  test_sink.StartCapturingLogs();
  LOG(WARNING)
          .AtLocation("fake/file", 123)
          .NoPrefix()
          .WithTimestamp(turbo::UnixEpoch())
          .WithThreadID(456)
          .WithVerbosity(7)
          .ToSinkOnly(&forwarding_sink)
      << "hello world";
}

TEST(TailCallsModifiesTest, WithPerror) {
  turbo::ScopedMockLog test_sink(turbo::MockLogDefault::kDisallowUnexpected);

  EXPECT_CALL(
      test_sink,
      Send(AllOf(TextMessage(AnyOf(Eq("hello world: Bad file number [9]"),
                                   Eq("hello world: Bad file descriptor [9]"),
                                   Eq("hello world: Bad file descriptor [8]"))),
                 ENCODED_MESSAGE(
                     AnyOf(EqualsProto(R"pb(value { literal: "hello world" }
                                            value { literal: ": " }
                                            value { str: "Bad file number" }
                                            value { literal: " [" }
                                            value { str: "9" }
                                            value { literal: "]" })pb"),
                           EqualsProto(R"pb(value { literal: "hello world" }
                                            value { literal: ": " }
                                            value { str: "Bad file descriptor" }
                                            value { literal: " [" }
                                            value { str: "9" }
                                            value { literal: "]" })pb"),
                           EqualsProto(R"pb(value { literal: "hello world" }
                                            value { literal: ": " }
                                            value { str: "Bad file descriptor" }
                                            value { literal: " [" }
                                            value { str: "8" }
                                            value { literal: "]" })pb"))))));

  test_sink.StartCapturingLogs();
  errno = EBADF;
  LOG(INFO).WithPerror() << "hello world";
}

#if GTEST_HAS_DEATH_TEST
TEST(ModifierMethodDeathTest, ToSinkOnlyQFatal) {
  EXPECT_EXIT(
      {
        turbo::ScopedMockLog test_sink(
            turbo::MockLogDefault::kDisallowUnexpected);

        auto do_log = [&test_sink] {
          LOG(QFATAL).ToSinkOnly(&test_sink.UseAsLocalSink()) << "hello world";
        };

        EXPECT_CALL(test_sink, Send)
            .Times(AnyNumber())
            .WillRepeatedly(DeathTestUnexpectedLogging());

        EXPECT_CALL(test_sink, Send(AllOf(TextMessage(Eq("hello world")),
                                          Stacktrace(IsEmpty()))))
            .WillOnce(DeathTestExpectedLogging());

        test_sink.StartCapturingLogs();
        do_log();
      },
      DiedOfQFatal, DeathTestValidateExpectations());
}
#endif

}  // namespace
