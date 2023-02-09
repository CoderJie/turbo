// Copyright 2022 The Turbo Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <cstdint>
#include <memory>

#include "turbo/crc/crc32c.h"
#include "turbo/crc/internal/crc_memcpy.h"
#include "turbo/platform/port.h"

namespace turbo {
TURBO_NAMESPACE_BEGIN
namespace crc_internal {

turbo::crc32c_t FallbackCrcMemcpyEngine::Compute(void* __restrict dst,
                                                const void* __restrict src,
                                                std::size_t length,
                                                crc32c_t initial_crc) const {
  constexpr size_t kBlockSize = 8192;
  turbo::crc32c_t crc = initial_crc;

  const char* src_bytes = reinterpret_cast<const char*>(src);
  char* dst_bytes = reinterpret_cast<char*>(dst);

  // Copy + CRC loop - run 8k chunks until we are out of full chunks.  CRC
  // then copy was found to be slightly more efficient in our test cases.
  std::size_t offset = 0;
  for (; offset + kBlockSize < length; offset += kBlockSize) {
    crc = turbo::ExtendCrc32c(crc,
                             turbo::string_view(src_bytes + offset, kBlockSize));
    memcpy(dst_bytes + offset, src_bytes + offset, kBlockSize);
  }

  // Save some work if length is 0.
  if (offset < length) {
    std::size_t final_copy_size = length - offset;
    crc = turbo::ExtendCrc32c(
        crc, turbo::string_view(src_bytes + offset, final_copy_size));
    memcpy(dst_bytes + offset, src_bytes + offset, final_copy_size);
  }

  return crc;
}

// Compile the following only if we don't have
#ifndef TURBO_INTERNAL_HAVE_X86_64_ACCELERATED_CRC_MEMCPY_ENGINE

CrcMemcpy::ArchSpecificEngines CrcMemcpy::GetArchSpecificEngines() {
  CrcMemcpy::ArchSpecificEngines engines;
  engines.temporal = new FallbackCrcMemcpyEngine();
  engines.non_temporal = new FallbackCrcMemcpyEngine();
  return engines;
}

std::unique_ptr<CrcMemcpyEngine> CrcMemcpy::GetTestEngine(int /*vector*/,
                                                          int /*integer*/) {
  return std::make_unique<FallbackCrcMemcpyEngine>();
}

#endif  // TURBO_INTERNAL_HAVE_X86_64_ACCELERATED_CRC_MEMCPY_ENGINE

}  // namespace crc_internal
TURBO_NAMESPACE_END
}  // namespace turbo
