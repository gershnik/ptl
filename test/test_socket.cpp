// Copyright (c) 2023, Eugene Gershnik
// SPDX-License-Identifier: BSD-3-Clause

#include <ptl/socket.h>

#include <catch2/catch_test_macros.hpp>

#include "common.h"

using namespace ptl;

TEST_CASE( "socket creation" , "[socket]") {

    auto sock = createSocket(PF_INET, SOCK_STREAM, 0);
    CHECK(sock);

    CHECK_THROWS_MATCHES(createSocket(PF_UNIX, SOCK_DGRAM, 0), std::system_error, EqualsSystemError(std::errc::address_family_not_supported));
}

TEST_CASE( "socket options" , "[socket]") {

    auto sock = createSocket(PF_INET, SOCK_STREAM, 0);
    REQUIRE(sock);

    setSocketOption(sock, SockOptDebug{true});
    CHECK(getSocketOption<SockOptDebug>(sock).value());

    CHECK_THROWS_MATCHES(setSocketOption(sock, SockOptBroadcast{true}), std::system_error, EqualsSystemError(std::errc::no_protocol_option));
}