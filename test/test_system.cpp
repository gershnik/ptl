// Copyright (c) 2023, Eugene Gershnik
// SPDX-License-Identifier: BSD-3-Clause

#include <ptl/system.h>

#include <catch2/catch_test_macros.hpp>

#include "common.h"

using namespace ptl;

#ifndef _WIN32

TEST_CASE( "sysconf" , "[system]") {

    auto res = ptl::systemConfig(_SC_OPEN_MAX);
    REQUIRE(res);
    CHECK(*res >= _POSIX_OPEN_MAX);

    CHECK_THROWS_MATCHES(ptl::systemConfig(32765), std::system_error, EqualsSystemError(std::errc::invalid_argument));
}

TEST_CASE( "gethostname" , "[system]") {

    auto hostname = shell({"hostname"});


    auto len = ptl::systemConfig(_SC_HOST_NAME_MAX).value_or(_POSIX_HOST_NAME_MAX);

    std::string str(1, 'A');
    std::error_code ec;
    ptl::getHostName(str, ec);
    CHECK((ec.value() == ENAMETOOLONG || str[str.size() - 1] == 0));
    
    str.resize(3, 'A');
    ptl::getHostName(str, ec);
    CHECK((ec.value() == ENAMETOOLONG || str[str.size() - 1] == 0));

    str.resize(len + 1);
    ptl::getHostName(str, ec);
    CHECK(!ec);
    str.resize(strlen(str.c_str()));
    CHECK(str == hostname);
}


#endif