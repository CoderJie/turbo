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

#include "call_once.h"

#include <thread>
#include <vector>

#include "turbo/base/const_init.h"
#include "turbo/platform/port.h"
#include "turbo/platform/thread_annotations.h"
#include "turbo/synchronization/mutex.h"
#include "gtest/gtest.h"

namespace turbo {
TURBO_NAMESPACE_BEGIN
namespace {

turbo::once_flag once;

TURBO_CONST_INIT Mutex counters_mu(turbo::kConstInit);

int running_thread_count TURBO_GUARDED_BY(counters_mu) = 0;
int call_once_invoke_count TURBO_GUARDED_BY(counters_mu) = 0;
int call_once_finished_count TURBO_GUARDED_BY(counters_mu) = 0;
int call_once_return_count TURBO_GUARDED_BY(counters_mu) = 0;
bool done_blocking TURBO_GUARDED_BY(counters_mu) = false;

// Function to be called from turbo::call_once.  Waits for a notification.
void WaitAndIncrement() {
  counters_mu.Lock();
  ++call_once_invoke_count;
  counters_mu.Unlock();

  counters_mu.LockWhen(Condition(&done_blocking));
  ++call_once_finished_count;
  counters_mu.Unlock();
}

void ThreadBody() {
  counters_mu.Lock();
  ++running_thread_count;
  counters_mu.Unlock();

  turbo::call_once(once, WaitAndIncrement);

  counters_mu.Lock();
  ++call_once_return_count;
  counters_mu.Unlock();
}

// Returns true if all threads are set up for the test.
bool ThreadsAreSetup(void*) TURBO_EXCLUSIVE_LOCKS_REQUIRED(counters_mu) {
  // All ten threads must be running, and WaitAndIncrement should be blocked.
  return running_thread_count == 10 && call_once_invoke_count == 1;
}

TEST(CallOnceTest, ExecutionCount) {
  std::vector<std::thread> threads;

  // Start 10 threads all calling call_once on the same once_flag.
  for (int i = 0; i < 10; ++i) {
    threads.emplace_back(ThreadBody);
  }


  // Wait until all ten threads have started, and WaitAndIncrement has been
  // invoked.
  counters_mu.LockWhen(Condition(ThreadsAreSetup, nullptr));

  // WaitAndIncrement should have been invoked by exactly one call_once()
  // instance.  That thread should be blocking on a notification, and all other
  // call_once instances should be blocking as well.
  EXPECT_EQ(call_once_invoke_count, 1);
  EXPECT_EQ(call_once_finished_count, 0);
  EXPECT_EQ(call_once_return_count, 0);

  // Allow WaitAndIncrement to finish executing.  Once it does, the other
  // call_once waiters will be unblocked.
  done_blocking = true;
  counters_mu.Unlock();

  for (std::thread& thread : threads) {
    thread.join();
  }

  counters_mu.Lock();
  EXPECT_EQ(call_once_invoke_count, 1);
  EXPECT_EQ(call_once_finished_count, 1);
  EXPECT_EQ(call_once_return_count, 10);
  counters_mu.Unlock();
}

}  // namespace
TURBO_NAMESPACE_END
}  // namespace turbo
