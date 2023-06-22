#include <ptl/file_descriptor.h>

#include <catch2/catch_test_macros.hpp>

#include "common.h"

TEST_CASE( "open file" , "[file_descriptor]") {

    CHECK_THROWS_MATCHES([]() {
        auto fd = ptl::FileDescriptor::open("nonexistant", O_RDONLY);
    }(), std::system_error, EqualsSystemError(std::errc::no_such_file_or_directory));

    std::error_code ec;
    auto fd = ptl::FileDescriptor::open("nonexistant", O_RDONLY, ec);
    CHECK(!fd);
    CHECK(ec.value() == ENOENT);
}

