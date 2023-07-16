// Copyright (c) 2023, Eugene Gershnik
// SPDX-License-Identifier: BSD-3-Clause

#include <ptl/socket.h>

#include <catch2/catch_test_macros.hpp>

#include "common.h"

using namespace ptl;

TEST_CASE( "socket creation" , "[socket]") {

    auto sock = createSocket(PF_INET, SOCK_STREAM, 0);
    CHECK(sock);

    try {
        sock = createSocket(PF_UNIX, SOCK_DGRAM, 0);
        CHECK(sock);
    } catch(std::system_error & ex) {
        CHECK_THAT(ex, equalsSystemError(std::errc::address_family_not_supported));
    }
}

TEST_CASE( "socket options" , "[socket]") {

    auto sock = createSocket(PF_INET, SOCK_STREAM, 0);
    REQUIRE(sock);

    {
        INFO("SockOptDebug");
        setSocketOption(sock, SockOptDebug{true});
        CHECK(getSocketOption<SockOptDebug>(sock).value());
    }

    {
        INFO("SockOptBroadcast");
        std::error_code ec;
        setSocketOption(sock, SockOptBroadcast{true}, ec);
        if (ec)
            CHECK(errorEquals(ec, std::errc::no_protocol_option));
        else
            CHECK(getSocketOption<SockOptBroadcast>(sock).value());
    }

    {
        INFO("SockOptReuseAddr");
        setSocketOption(sock, SockOptReuseAddr{true});
        CHECK(getSocketOption<SockOptReuseAddr>(sock).value());
    }

    {
        INFO("SockOptKeepAlive");
        setSocketOption(sock, SockOptKeepAlive{true});
        CHECK(getSocketOption<SockOptKeepAlive>(sock).value());
    }

    {
        INFO("SockOptLinger");
        SockOptLinger linger;
        linger.l_onoff = 1;
        linger.l_linger = 3;
        setSocketOption(sock, linger);
        SockOptLinger lingerOut;
        CHECK((lingerOut.l_onoff == 0 && lingerOut.l_linger == 0));
        getSocketOption(sock, lingerOut);
        CHECK((linger.l_onoff == lingerOut.l_onoff && linger.l_linger == lingerOut.l_linger));
    }
}