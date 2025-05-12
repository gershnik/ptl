// Copyright (c) 2023, Eugene Gershnik
// SPDX-License-Identifier: BSD-3-Clause

#include <ptl/system.h>

#include <limits.h>
#include <string.h>

#include "common.h"

using namespace ptl;

#ifndef _WIN32

TEST_SUITE("system") {

TEST_CASE( "sysconf" ) {

    auto res = ptl::systemConfig(_SC_OPEN_MAX);
    REQUIRE(res);
    CHECK(*res >= _POSIX_OPEN_MAX);

    CHECK_THROWS_MATCHES(ptl::systemConfig(32765), std::errc::invalid_argument);
}

#if !defined(__EMSCRIPTEN__)

TEST_CASE( "gethostname" ) {

    auto hostname = shell({"hostname"});


    auto len = ptl::systemConfig(_SC_HOST_NAME_MAX).value_or(_POSIX_HOST_NAME_MAX);

    std::string str(1, 'A');
    std::error_code ec;
    ptl::getHostName(str, ec);
    CHECK((errorEquals(ec, std::errc::filename_too_long) || str[str.size() - 1] == 0));
    
    str.resize(3, 'A');
    ptl::getHostName(str, ec);
    CHECK((errorEquals(ec, std::errc::filename_too_long) || str[str.size() - 1] == 0));

    str.resize(len + 1);
    ptl::getHostName(str, ec);
    CHECK(!ec);
    str.resize(strlen(str.c_str()));
    CHECK(str == hostname);
}
#endif

}

#endif