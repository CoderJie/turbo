// This test file is part of the fwd_test.cpp/impl_test.cpp pair
// and used to test the new optional two-part usage of turbo::filesystem
// where exactly one cpp includes fs_impl.hpp and all others use
// fs_fwd.hpp (to test this with maximum functionality, the unit tests
// are included here, signaling they should only include the fs_fwd.hpp)
#define TURBO_FILESYSTEM_FWD_TEST
#include "filesystem_test.cc"
