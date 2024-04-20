// Copyright (c) 2023, Eugene Gershnik
// SPDX-License-Identifier: BSD-3-Clause

#include <ptl/errors.h>
#include <ptl/file.h>

#include "common.h"

using namespace ptl;

TEST_SUITE("errors") {

TEST_CASE( "allowed" ) {

    {
        AllowedErrors<ENOENT, EDOM> ec;
        auto fd = FileDescriptor::open("nonexistant", O_RDONLY, ec);
        CHECK(ec.code() == ENOENT);
    }

    {
        CHECK_THROWS_MATCHES([]() {
            AllowedErrors<EDOM> ec;
            auto fd = FileDescriptor::open("nonexistant", O_RDONLY, ec);
        }(), std::errc::no_such_file_or_directory);
    }
}

}