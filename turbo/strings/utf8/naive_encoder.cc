// Copyright 2022 ByteDance Ltd. and/or its affiliates.
// Copyright 2022 The Turbo Authors
/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#include "turbo/strings/utf8/naive_encoder.h"

#include <cstring>
#include <string>
#include "turbo/base/casts.h"

namespace turbo {
TURBO_NAMESPACE_BEGIN
namespace utf8_details {

static constexpr uint32_t UTF8_BUF_SIZE = 8;
static constexpr uint32_t TURBO_ALLOW_UNUSED UTF8_MAX = 0x7FFFFFFFu;
// https://unicodebook.readthedocs.io/unicode_encodings.html
static constexpr uint32_t UNICODE_MAX = 0x10FFFF;

static constexpr const char* const invalid_char = "�";
static const size_t invalid_char_len = std::char_traits<char>::length(invalid_char);

static inline size_t NaiveOneCharLen(uint32_t x) noexcept {
  if (x < 0x80) {
    /* ascii? */
    return 1;
  } else if (x <= 0x7FF) {
    return 2;
  } else if (x <= 0xFFFF) {
    return 3;
  } else if (x <= UNICODE_MAX) {
    return 4;
  }
  return invalid_char_len;
}

size_t NaiveCountBytesSize(const uint32_t* s_ptr, const uint32_t* s_ptr_end) noexcept {
  size_t counts = 0;
  while (s_ptr < s_ptr_end) {
    counts += NaiveOneCharLen(*s_ptr);
    ++s_ptr;
  }
  return counts;
}


static int32_t NaiveUTF8EncodeOne(char* buff, uint32_t x) noexcept {
  TURBO_DISABLE_CLANG_WARNING(-Wsign-conversion);
  int n = 1; /* number of bytes put in buffer (backwards) */
  if (x < 0x80) {
    /* ascii? */
    buff[UTF8_BUF_SIZE - 1] = x & 0x7F;
  } else {               /* need continuation bytes */
    uint32_t mfb = 0x3f; /* maximum that fits in first byte */
    do {                 /* add continuation bytes */
      buff[UTF8_BUF_SIZE - static_cast<size_t>(n++)] = 0x80 |( x & 0x3f);
      x >>= 6;         /* remove added bits */
      mfb >>= 1;       /* now there is one less bit available in first byte */
    } while (x > mfb); /* still needs continuation byte? */
    buff[UTF8_BUF_SIZE - n] = ((~mfb << 1) | x) & 0xFF; /* add first byte */
  }
    TURBO_RESTORE_CLANG_WARNING();
  return n;
}

ptrdiff_t NaiveEncoder(const uint32_t* s_ptr,
                       const uint32_t* s_ptr_end,
                       unsigned char* dst) noexcept {
  auto* dst_origin = dst;
  while (s_ptr < s_ptr_end) {
    auto code_point = *s_ptr;
    ++s_ptr;
    char buff[UTF8_BUF_SIZE];
    if (code_point <= UNICODE_MAX) {
      auto n = NaiveUTF8EncodeOne(buff, code_point);
      std::memcpy(dst, buff + UTF8_BUF_SIZE - n, static_cast<size_t>(n));
      dst += n;
    } else {
      std::memcpy(dst, invalid_char, invalid_char_len);
      dst += invalid_char_len;
    }
  }
  return dst - dst_origin;
}

}  // namespace utf8_details
TURBO_NAMESPACE_END
}  // namespace turbo
