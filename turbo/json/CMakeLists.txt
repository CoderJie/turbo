
file(COPY bin DESTINATION ${PROJECT_BINARY_DIR})

set(JSON_SOURCES
	allocators_test.cc
    biginteger_test.cc
    clzll_test.cc
	cursorstreamwrapper_test.cc
    document_test.cc
    dtoa_test.cc
    encodedstream_test.cc
    encodings_test.cc
    fwd_test.cc
    filestream_test.cc
    itoa_test.cc
    istreamwrapper_test.cc
    jsonchecker_test.cc
    namespace_test.cc
    pointer_test.cc
    platform_test.cc
    prettywriter_test.cc
    ostreamwrapper_test.cc
    reader_test.cc
    regex_test.cc
	schema_test.cc
	simd_test.cc
    strfunc_test.cc
    stringbuffer_test.cc
    strtod_test.cc
    unit_test.cc
    uri_test.cc
    value_test.cc
    writer_test.cc)

turbo_cc_test(
        NAME
        json_test
        SRCS
        ${JSON_SOURCES}
        COPTS
        ${TURBO_TEST_COPTS}
        DEPS
        turbo::turbo
        GTest::gtest
        GTest::gtest_main
)


turbo_cc_test(
		NAME
		robust_json_test
		SRCS
		"robust_json_test.cc"
		COPTS
		${TURBO_TEST_COPTS}
		DEPS
		turbo::turbo
		GTest::gtest
		GTest::gtest_main
)