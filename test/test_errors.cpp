// Copyright (c) 2023, Eugene Gershnik
// SPDX-License-Identifier: BSD-3-Clause

#include <ptl/errors.h>
#include <ptl/file.h>

#include <catch2/catch_test_macros.hpp>

#include "common.h"

using namespace ptl;

TEST_CASE( "allowed" , "[errors]") {

    {
        AllowedErrors<ENOENT, EDOM> ec;
        auto fd = FileDescriptor::open("nonexistant", O_RDONLY, ec);
        CHECK(errorEquals(ec.code(), std::errc::no_such_file_or_directory));
    }

    {
        CHECK_THROWS_MATCHES([]() {
            AllowedErrors<EDOM> ec;
            auto fd = FileDescriptor::open("nonexistant", O_RDONLY, ec);
        }(), std::system_error, equalsSystemError(std::errc::no_such_file_or_directory));
    }



}