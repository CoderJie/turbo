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

#include "turbo/platform/port.h"
#include "turbo/strings/string_view.h"
#include "gtest/gtest.h"

// This test by itself does not do anything fancy, but it serves as binary I can
// query in shell test.

namespace {

template <class T>
void DoNotOptimize(const T& var) {
#ifdef __GNUC__
  asm volatile("" : "+m"(const_cast<T&>(var)));
#else
  std::cout << (void*)&var;
#endif
}

int very_long_int_variable_name TURBO_INTERNAL_UNIQUE_SMALL_NAME() = 0;
char very_long_str_variable_name[] TURBO_INTERNAL_UNIQUE_SMALL_NAME() = "abc";

TEST(UniqueSmallName, NonAutomaticVar) {
  EXPECT_EQ(very_long_int_variable_name, 0);
  EXPECT_EQ(turbo::string_view(very_long_str_variable_name), "abc");
}

int VeryLongFreeFunctionName() TURBO_INTERNAL_UNIQUE_SMALL_NAME();

TEST(UniqueSmallName, FreeFunction) {
  DoNotOptimize(&VeryLongFreeFunctionName);

  EXPECT_EQ(VeryLongFreeFunctionName(), 456);
}

int VeryLongFreeFunctionName() { return 456; }

struct VeryLongStructName {
  explicit VeryLongStructName(int i);

  int VeryLongMethodName() TURBO_INTERNAL_UNIQUE_SMALL_NAME();

  static int VeryLongStaticMethodName() TURBO_INTERNAL_UNIQUE_SMALL_NAME();

 private:
  int fld;
};

TEST(UniqueSmallName, Struct) {
  VeryLongStructName var(10);

  DoNotOptimize(var);
  DoNotOptimize(&VeryLongStructName::VeryLongMethodName);
  DoNotOptimize(&VeryLongStructName::VeryLongStaticMethodName);

  EXPECT_EQ(var.VeryLongMethodName(), 10);
  EXPECT_EQ(VeryLongStructName::VeryLongStaticMethodName(), 123);
}

VeryLongStructName::VeryLongStructName(int i) : fld(i) {}
int VeryLongStructName::VeryLongMethodName() { return fld; }
int VeryLongStructName::VeryLongStaticMethodName() { return 123; }

}  // namespace
