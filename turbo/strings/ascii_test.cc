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

#include "turbo/strings/ascii.h"

#include <cctype>
#include <clocale>
#include <cstring>
#include <string>

#include "turbo/platform/port.h"
#include "gtest/gtest.h"
#include "turbo/strings/inlined_string.h"

namespace {

TEST(AsciiIsFoo, All) {
  for (int i = 0; i < 256; i++) {
    const auto c = static_cast<unsigned char>(i);
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
      EXPECT_TRUE(turbo::ascii_isalpha(c)) << ": failed on " << c;
    else
      EXPECT_TRUE(!turbo::ascii_isalpha(c)) << ": failed on " << c;
  }
  for (int i = 0; i < 256; i++) {
    const auto c = static_cast<unsigned char>(i);
    if ((c >= '0' && c <= '9'))
      EXPECT_TRUE(turbo::ascii_isdigit(c)) << ": failed on " << c;
    else
      EXPECT_TRUE(!turbo::ascii_isdigit(c)) << ": failed on " << c;
  }
  for (int i = 0; i < 256; i++) {
    const auto c = static_cast<unsigned char>(i);
    if (turbo::ascii_isalpha(c) || turbo::ascii_isdigit(c))
      EXPECT_TRUE(turbo::ascii_isalnum(c)) << ": failed on " << c;
    else
      EXPECT_TRUE(!turbo::ascii_isalnum(c)) << ": failed on " << c;
  }
  for (int i = 0; i < 256; i++) {
    const auto c = static_cast<unsigned char>(i);
    if (i != '\0' && strchr(" \r\n\t\v\f", i))
      EXPECT_TRUE(turbo::ascii_isspace(c)) << ": failed on " << c;
    else
      EXPECT_TRUE(!turbo::ascii_isspace(c)) << ": failed on " << c;
  }
  for (int i = 0; i < 256; i++) {
    const auto c = static_cast<unsigned char>(i);
    if (i >= 32 && i < 127)
      EXPECT_TRUE(turbo::ascii_isprint(c)) << ": failed on " << c;
    else
      EXPECT_TRUE(!turbo::ascii_isprint(c)) << ": failed on " << c;
  }
  for (int i = 0; i < 256; i++) {
    const auto c = static_cast<unsigned char>(i);
    if (turbo::ascii_isprint(c) && !turbo::ascii_isspace(c) &&
        !turbo::ascii_isalnum(c)) {
      EXPECT_TRUE(turbo::ascii_ispunct(c)) << ": failed on " << c;
    } else {
      EXPECT_TRUE(!turbo::ascii_ispunct(c)) << ": failed on " << c;
    }
  }
  for (int i = 0; i < 256; i++) {
    const auto c = static_cast<unsigned char>(i);
    if (i == ' ' || i == '\t')
      EXPECT_TRUE(turbo::ascii_isblank(c)) << ": failed on " << c;
    else
      EXPECT_TRUE(!turbo::ascii_isblank(c)) << ": failed on " << c;
  }
  for (int i = 0; i < 256; i++) {
    const auto c = static_cast<unsigned char>(i);
    if (i < 32 || i == 127)
      EXPECT_TRUE(turbo::ascii_iscntrl(c)) << ": failed on " << c;
    else
      EXPECT_TRUE(!turbo::ascii_iscntrl(c)) << ": failed on " << c;
  }
  for (int i = 0; i < 256; i++) {
    const auto c = static_cast<unsigned char>(i);
    if (turbo::ascii_isdigit(c) || (i >= 'A' && i <= 'F') ||
        (i >= 'a' && i <= 'f')) {
      EXPECT_TRUE(turbo::ascii_isxdigit(c)) << ": failed on " << c;
    } else {
      EXPECT_TRUE(!turbo::ascii_isxdigit(c)) << ": failed on " << c;
    }
  }
  for (int i = 0; i < 256; i++) {
    const auto c = static_cast<unsigned char>(i);
    if (i > 32 && i < 127)
      EXPECT_TRUE(turbo::ascii_isgraph(c)) << ": failed on " << c;
    else
      EXPECT_TRUE(!turbo::ascii_isgraph(c)) << ": failed on " << c;
  }
  for (int i = 0; i < 256; i++) {
    const auto c = static_cast<unsigned char>(i);
    if (i >= 'A' && i <= 'Z')
      EXPECT_TRUE(turbo::ascii_isupper(c)) << ": failed on " << c;
    else
      EXPECT_TRUE(!turbo::ascii_isupper(c)) << ": failed on " << c;
  }
  for (int i = 0; i < 256; i++) {
    const auto c = static_cast<unsigned char>(i);
    if (i >= 'a' && i <= 'z')
      EXPECT_TRUE(turbo::ascii_islower(c)) << ": failed on " << c;
    else
      EXPECT_TRUE(!turbo::ascii_islower(c)) << ": failed on " << c;
  }
  for (unsigned char c = 0; c < 128; c++) {
    EXPECT_TRUE(turbo::ascii_isascii(c)) << ": failed on " << c;
  }
  for (int i = 128; i < 256; i++) {
    const auto c = static_cast<unsigned char>(i);
    EXPECT_TRUE(!turbo::ascii_isascii(c)) << ": failed on " << c;
  }
}

// Checks that turbo::ascii_isfoo returns the same value as isfoo in the C
// locale.
TEST(AsciiIsFoo, SameAsIsFoo) {
#ifndef __ANDROID__
  // temporarily change locale to C. It should already be C, but just for safety
  const char* old_locale = setlocale(LC_CTYPE, "C");
  ASSERT_TRUE(old_locale != nullptr);
#endif

  for (int i = 0; i < 256; i++) {
    const auto c = static_cast<unsigned char>(i);
    EXPECT_EQ(isalpha(c) != 0, turbo::ascii_isalpha(c)) << c;
    EXPECT_EQ(isdigit(c) != 0, turbo::ascii_isdigit(c)) << c;
    EXPECT_EQ(isalnum(c) != 0, turbo::ascii_isalnum(c)) << c;
    EXPECT_EQ(isspace(c) != 0, turbo::ascii_isspace(c)) << c;
    EXPECT_EQ(ispunct(c) != 0, turbo::ascii_ispunct(c)) << c;
    EXPECT_EQ(isblank(c) != 0, turbo::ascii_isblank(c)) << c;
    EXPECT_EQ(iscntrl(c) != 0, turbo::ascii_iscntrl(c)) << c;
    EXPECT_EQ(isxdigit(c) != 0, turbo::ascii_isxdigit(c)) << c;
    EXPECT_EQ(isprint(c) != 0, turbo::ascii_isprint(c)) << c;
    EXPECT_EQ(isgraph(c) != 0, turbo::ascii_isgraph(c)) << c;
    EXPECT_EQ(isupper(c) != 0, turbo::ascii_isupper(c)) << c;
    EXPECT_EQ(islower(c) != 0, turbo::ascii_islower(c)) << c;
    EXPECT_EQ(isascii(c) != 0, turbo::ascii_isascii(c)) << c;
  }

#ifndef __ANDROID__
  // restore the old locale.
  ASSERT_TRUE(setlocale(LC_CTYPE, old_locale));
#endif
}

TEST(AsciiToFoo, All) {
#ifndef __ANDROID__
  // temporarily change locale to C. It should already be C, but just for safety
  const char* old_locale = setlocale(LC_CTYPE, "C");
  ASSERT_TRUE(old_locale != nullptr);
#endif

  for (int i = 0; i < 256; i++) {
    const auto c = static_cast<unsigned char>(i);
    if (turbo::ascii_islower(c))
      EXPECT_EQ(turbo::ascii_toupper(c), 'A' + (i - 'a')) << c;
    else
      EXPECT_EQ(turbo::ascii_toupper(c), static_cast<char>(i)) << c;

    if (turbo::ascii_isupper(c))
      EXPECT_EQ(turbo::ascii_tolower(c), 'a' + (i - 'A')) << c;
    else
      EXPECT_EQ(turbo::ascii_tolower(c), static_cast<char>(i)) << c;

    // These CHECKs only hold in a C locale.
    EXPECT_EQ(static_cast<char>(tolower(i)), turbo::ascii_tolower(c)) << c;
    EXPECT_EQ(static_cast<char>(toupper(i)), turbo::ascii_toupper(c)) << c;
  }
#ifndef __ANDROID__
  // restore the old locale.
  ASSERT_TRUE(setlocale(LC_CTYPE, old_locale));
#endif
}

TEST(AsciiStrTo, Lower) {
  const char buf[] = "ABCDEF";
  const std::string str("GHIJKL");
  const std::string str2("MNOPQR");
  const turbo::string_view sp(str2);
  std::string mutable_str("STUVWX");

  EXPECT_EQ("abcdef", turbo::AsciiStrToLower(buf));
  EXPECT_EQ("ghijkl", turbo::AsciiStrToLower(str));
  EXPECT_EQ("mnopqr", turbo::AsciiStrToLower(sp));

  turbo::AsciiStrToLower(&mutable_str);
  EXPECT_EQ("stuvwx", mutable_str);

  char mutable_buf[] = "Mutable";
  std::transform(mutable_buf, mutable_buf + strlen(mutable_buf),
                 mutable_buf, turbo::ascii_tolower);
  EXPECT_STREQ("mutable", mutable_buf);
}

TEST(AsciiStrTo, Upper) {
  const char buf[] = "abcdef";
  const std::string str("ghijkl");
  const std::string str2("mnopqr");
  const turbo::string_view sp(str2);

  EXPECT_EQ("ABCDEF", turbo::AsciiStrToUpper(buf));
  EXPECT_EQ("GHIJKL", turbo::AsciiStrToUpper(str));
  EXPECT_EQ("MNOPQR", turbo::AsciiStrToUpper(sp));

  char mutable_buf[] = "Mutable";
  std::transform(mutable_buf, mutable_buf + strlen(mutable_buf),
                 mutable_buf, turbo::ascii_toupper);
  EXPECT_STREQ("MUTABLE", mutable_buf);
}

TEST(StripLeadingAsciiWhitespace, FromStringView) {
  EXPECT_EQ(turbo::string_view{},
            turbo::StripLeadingAsciiWhitespace(turbo::string_view{}));
  EXPECT_EQ("foo", turbo::StripLeadingAsciiWhitespace({"foo"}));
  EXPECT_EQ("foo", turbo::StripLeadingAsciiWhitespace({"\t  \n\f\r\n\vfoo"}));
  EXPECT_EQ("foo foo\n ",
            turbo::StripLeadingAsciiWhitespace({"\t  \n\f\r\n\vfoo foo\n "}));
  EXPECT_EQ(turbo::string_view{}, turbo::StripLeadingAsciiWhitespace(
                                     {"\t  \n\f\r\v\n\t  \n\f\r\v\n"}));
}

template<typename Str>
void TestInPlace() {
  Str str;

  turbo::StripLeadingAsciiWhitespace(&str);
  EXPECT_EQ("", str);

  str = "foo";
  turbo::StripLeadingAsciiWhitespace(&str);
  EXPECT_EQ("foo", str);

  str = "\t  \n\f\r\n\vfoo";
  turbo::StripLeadingAsciiWhitespace(&str);
  EXPECT_EQ("foo", str);

  str = "\t  \n\f\r\n\vfoo foo\n ";
  turbo::StripLeadingAsciiWhitespace(&str);
  EXPECT_EQ("foo foo\n ", str);

  str = "\t  \n\f\r\v\n\t  \n\f\r\v\n";
  turbo::StripLeadingAsciiWhitespace(&str);
  EXPECT_EQ(turbo::string_view{}, str);
}
TEST(StripLeadingAsciiWhitespace, InPlace) {
    TestInPlace<std::string>();
    TestInPlace<turbo::inlined_string>();
}

TEST(StripTrailingAsciiWhitespace, FromStringView) {
  EXPECT_EQ(turbo::string_view{},
            turbo::StripTrailingAsciiWhitespace(turbo::string_view{}));
  EXPECT_EQ("foo", turbo::StripTrailingAsciiWhitespace({"foo"}));
  EXPECT_EQ("foo", turbo::StripTrailingAsciiWhitespace({"foo\t  \n\f\r\n\v"}));
  EXPECT_EQ(" \nfoo foo",
            turbo::StripTrailingAsciiWhitespace({" \nfoo foo\t  \n\f\r\n\v"}));
  EXPECT_EQ(turbo::string_view{}, turbo::StripTrailingAsciiWhitespace(
                                     {"\t  \n\f\r\v\n\t  \n\f\r\v\n"}));
}

template <typename String>
void StripTrailingAsciiWhitespaceinplace() {
  String str;

  turbo::StripTrailingAsciiWhitespace(&str);
  EXPECT_EQ("", str);

  str = "foo";
  turbo::StripTrailingAsciiWhitespace(&str);
  EXPECT_EQ("foo", str);

  str = "foo\t  \n\f\r\n\v";
  turbo::StripTrailingAsciiWhitespace(&str);
  EXPECT_EQ("foo", str);

  str = " \nfoo foo\t  \n\f\r\n\v";
  turbo::StripTrailingAsciiWhitespace(&str);
  EXPECT_EQ(" \nfoo foo", str);

  str = "\t  \n\f\r\v\n\t  \n\f\r\v\n";
  turbo::StripTrailingAsciiWhitespace(&str);
  EXPECT_EQ(turbo::string_view{}, str);
}
TEST(StripTrailingAsciiWhitespace, InPlace) {
    StripTrailingAsciiWhitespaceinplace<std::string>();
    StripTrailingAsciiWhitespaceinplace<turbo::inlined_string>();
}

TEST(StripAsciiWhitespace, FromStringView) {
  EXPECT_EQ(turbo::string_view{},
            turbo::StripAsciiWhitespace(turbo::string_view{}));
  EXPECT_EQ("foo", turbo::StripAsciiWhitespace({"foo"}));
  EXPECT_EQ("foo",
            turbo::StripAsciiWhitespace({"\t  \n\f\r\n\vfoo\t  \n\f\r\n\v"}));
  EXPECT_EQ("foo foo", turbo::StripAsciiWhitespace(
                           {"\t  \n\f\r\n\vfoo foo\t  \n\f\r\n\v"}));
  EXPECT_EQ(turbo::string_view{},
            turbo::StripAsciiWhitespace({"\t  \n\f\r\v\n\t  \n\f\r\v\n"}));
}

template <typename Str>
void StripAsciiWhitespaceInPlace() {
  Str str;

  turbo::StripAsciiWhitespace(&str);
  EXPECT_EQ("", str);

  str = "foo";
  turbo::StripAsciiWhitespace(&str);
  EXPECT_EQ("foo", str);

  str = "\t  \n\f\r\n\vfoo\t  \n\f\r\n\v";
  turbo::StripAsciiWhitespace(&str);
  EXPECT_EQ("foo", str);

  str = "\t  \n\f\r\n\vfoo foo\t  \n\f\r\n\v";
  turbo::StripAsciiWhitespace(&str);
  EXPECT_EQ("foo foo", str);

  str = "\t  \n\f\r\v\n\t  \n\f\r\v\n";
  turbo::StripAsciiWhitespace(&str);
  EXPECT_EQ(turbo::string_view{}, str);
}
TEST(StripAsciiWhitespace, InPlace) {
    StripAsciiWhitespaceInPlace<std::string>();
    StripAsciiWhitespaceInPlace<turbo::inlined_string>();
}

template <typename String>
void RemoveExtraAsciiWhitespaceInplace() {
    const char* inputs[] = {"No extra space",
                            "  Leading whitespace",
                            "Trailing whitespace  ",
                            "  Leading and trailing  ",
                            " Whitespace \t  in\v   middle  ",
                            "'Eeeeep!  \n Newlines!\n",
                            "nospaces",
                            "",
                            "\n\t a\t\n\nb \t\n"};

    const char* outputs[] = {
        "No extra space",
        "Leading whitespace",
        "Trailing whitespace",
        "Leading and trailing",
        "Whitespace in middle",
        "'Eeeeep! Newlines!",
        "nospaces",
        "",
        "a\nb",
    };
    const int NUM_TESTS = TURBO_ARRAY_SIZE(inputs);

    for (int i = 0; i < NUM_TESTS; i++) {
      String s(inputs[i]);
      turbo::RemoveExtraAsciiWhitespace(&s);
      EXPECT_EQ(outputs[i], s);
    }
}
TEST(RemoveExtraAsciiWhitespace, InPlace) {
    RemoveExtraAsciiWhitespaceInplace<std::string>();
    RemoveExtraAsciiWhitespaceInplace<turbo::inlined_string>();
}

}  // namespace
