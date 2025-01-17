//
//  Copyright 2019 The Turbo Authors.
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

#include "turbo/flags/marshalling.h"

#include <stddef.h>

#include <cmath>
#include <limits>
#include <string>
#include <type_traits>
#include <vector>

#include "turbo/base/log_severity.h"
#include "turbo/platform/port.h"
#include "turbo/strings/ascii.h"
#include "turbo/strings/match.h"
#include "turbo/strings/numbers.h"
#include "turbo/strings/str_cat.h"
#include "turbo/strings/str_format.h"
#include "turbo/strings/str_join.h"
#include "turbo/strings/str_split.h"
#include "turbo/strings/string_view.h"

namespace turbo {
TURBO_NAMESPACE_BEGIN
namespace flags_internal {

// --------------------------------------------------------------------
// TurboParseFlag specializations for boolean type.

bool TurboParseFlag(turbo::string_view text, bool* dst, std::string*) {
  const char* kTrue[] = {"1", "t", "true", "y", "yes"};
  const char* kFalse[] = {"0", "f", "false", "n", "no"};
  static_assert(sizeof(kTrue) == sizeof(kFalse), "true_false_equal");

  text = turbo::StripAsciiWhitespace(text);

  for (size_t i = 0; i < TURBO_ARRAY_SIZE(kTrue); ++i) {
    if (turbo::EqualsIgnoreCase(text, kTrue[i])) {
      *dst = true;
      return true;
    } else if (turbo::EqualsIgnoreCase(text, kFalse[i])) {
      *dst = false;
      return true;
    }
  }
  return false;  // didn't match a legal input
}

// --------------------------------------------------------------------
// TurboParseFlag for integral types.

// Return the base to use for parsing text as an integer.  Leading 0x
// puts us in base 16.  But leading 0 does not put us in base 8. It
// caused too many bugs when we had that behavior.
static int NumericBase(turbo::string_view text) {
  const bool hex = (text.size() >= 2 && text[0] == '0' &&
                    (text[1] == 'x' || text[1] == 'X'));
  return hex ? 16 : 10;
}

template <typename IntType>
inline bool ParseFlagImpl(turbo::string_view text, IntType& dst) {
  text = turbo::StripAsciiWhitespace(text);

  return turbo::numbers_internal::safe_strtoi_base(text, &dst,
                                                  NumericBase(text));
}

bool TurboParseFlag(turbo::string_view text, short* dst, std::string*) {
  int val;
  if (!ParseFlagImpl(text, val)) return false;
  if (static_cast<short>(val) != val)  // worked, but number out of range
    return false;
  *dst = static_cast<short>(val);
  return true;
}

bool TurboParseFlag(turbo::string_view text, unsigned short* dst, std::string*) {
  unsigned int val;
  if (!ParseFlagImpl(text, val)) return false;
  if (static_cast<unsigned short>(val) !=
      val)  // worked, but number out of range
    return false;
  *dst = static_cast<unsigned short>(val);
  return true;
}

bool TurboParseFlag(turbo::string_view text, int* dst, std::string*) {
  return ParseFlagImpl(text, *dst);
}

bool TurboParseFlag(turbo::string_view text, unsigned int* dst, std::string*) {
  return ParseFlagImpl(text, *dst);
}

bool TurboParseFlag(turbo::string_view text, long* dst, std::string*) {
  return ParseFlagImpl(text, *dst);
}

bool TurboParseFlag(turbo::string_view text, unsigned long* dst, std::string*) {
  return ParseFlagImpl(text, *dst);
}

bool TurboParseFlag(turbo::string_view text, long long* dst, std::string*) {
  return ParseFlagImpl(text, *dst);
}

bool TurboParseFlag(turbo::string_view text, unsigned long long* dst,
                   std::string*) {
  return ParseFlagImpl(text, *dst);
}

// --------------------------------------------------------------------
// TurboParseFlag for floating point types.

bool TurboParseFlag(turbo::string_view text, float* dst, std::string*) {
  return turbo::SimpleAtof(text, dst);
}

bool TurboParseFlag(turbo::string_view text, double* dst, std::string*) {
  return turbo::SimpleAtod(text, dst);
}

// --------------------------------------------------------------------
// TurboParseFlag for strings.

bool TurboParseFlag(turbo::string_view text, std::string* dst, std::string*) {
  dst->assign(text.data(), text.size());
  return true;
}

// --------------------------------------------------------------------
// TurboParseFlag for vector of strings.

bool TurboParseFlag(turbo::string_view text, std::vector<std::string>* dst,
                   std::string*) {
  // An empty flag value corresponds to an empty vector, not a vector
  // with a single, empty std::string.
  if (text.empty()) {
    dst->clear();
    return true;
  }
  *dst = turbo::StrSplit(text, ',', turbo::AllowEmpty());
  return true;
}

// --------------------------------------------------------------------
// TurboUnparseFlag specializations for various builtin flag types.

std::string Unparse(bool v) { return v ? "true" : "false"; }
std::string Unparse(short v) { return turbo::StrCat(v); }
std::string Unparse(unsigned short v) { return turbo::StrCat(v); }
std::string Unparse(int v) { return turbo::StrCat(v); }
std::string Unparse(unsigned int v) { return turbo::StrCat(v); }
std::string Unparse(long v) { return turbo::StrCat(v); }
std::string Unparse(unsigned long v) { return turbo::StrCat(v); }
std::string Unparse(long long v) { return turbo::StrCat(v); }
std::string Unparse(unsigned long long v) { return turbo::StrCat(v); }
template <typename T>
std::string UnparseFloatingPointVal(T v) {
  // digits10 is guaranteed to roundtrip correctly in string -> value -> string
  // conversions, but may not be enough to represent all the values correctly.
  std::string digit10_str =
      turbo::StrFormat("%.*g", std::numeric_limits<T>::digits10, v);
  if (std::isnan(v) || std::isinf(v)) return digit10_str;

  T roundtrip_val = 0;
  std::string err;
  if (turbo::ParseFlag(digit10_str, &roundtrip_val, &err) &&
      roundtrip_val == v) {
    return digit10_str;
  }

  // max_digits10 is the number of base-10 digits that are necessary to uniquely
  // represent all distinct values.
  return turbo::StrFormat("%.*g", std::numeric_limits<T>::max_digits10, v);
}
std::string Unparse(float v) { return UnparseFloatingPointVal(v); }
std::string Unparse(double v) { return UnparseFloatingPointVal(v); }
std::string TurboUnparseFlag(turbo::string_view v) { return std::string(v); }
std::string TurboUnparseFlag(const std::vector<std::string>& v) {
  return turbo::StrJoin(v, ",");
}

}  // namespace flags_internal

bool TurboParseFlag(turbo::string_view text, turbo::LogSeverity* dst,
                   std::string* err) {
  text = turbo::StripAsciiWhitespace(text);
  if (text.empty()) {
    *err = "no value provided";
    return false;
  }
  if (text.front() == 'k' || text.front() == 'K') text.remove_prefix(1);
  if (turbo::EqualsIgnoreCase(text, "info")) {
    *dst = turbo::LogSeverity::kInfo;
    return true;
  }
  if (turbo::EqualsIgnoreCase(text, "warning")) {
    *dst = turbo::LogSeverity::kWarning;
    return true;
  }
  if (turbo::EqualsIgnoreCase(text, "error")) {
    *dst = turbo::LogSeverity::kError;
    return true;
  }
  if (turbo::EqualsIgnoreCase(text, "fatal")) {
    *dst = turbo::LogSeverity::kFatal;
    return true;
  }
  std::underlying_type<turbo::LogSeverity>::type numeric_value;
  if (turbo::ParseFlag(text, &numeric_value, err)) {
    *dst = static_cast<turbo::LogSeverity>(numeric_value);
    return true;
  }
  *err = "only integers and turbo::LogSeverity enumerators are accepted";
  return false;
}

std::string TurboUnparseFlag(turbo::LogSeverity v) {
  if (v == turbo::NormalizeLogSeverity(v)) return turbo::LogSeverityName(v);
  return turbo::UnparseFlag(static_cast<int>(v));
}

TURBO_NAMESPACE_END
}  // namespace turbo
