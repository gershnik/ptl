// Copyright (c) 2023, Eugene Gershnik
// SPDX-License-Identifier: BSD-3-Clause

#include <ptl/socket.h>

#include "common.h"
#include <string.h>

#include <thread>

using namespace ptl;

#if !defined(__EMSCRIPTEN__)

TEST_SUITE("socket") {

TEST_CASE( "socket creation" ) {

    auto sock = createSocket(PF_INET, SOCK_STREAM, 0);
    CHECK(sock);

    try {
        sock = createSocket(PF_UNIX, SOCK_DGRAM, 0);
        CHECK(sock);
    } catch(std::system_error & ex) {
        ptl::errorEquals(ex.code(), std::errc::address_family_not_supported);
    }
}

TEST_CASE( "socket options" ) {

    auto sock = createSocket(PF_INET, SOCK_STREAM, 0);
    REQUIRE(sock);

    #ifndef __linux__
    {
        INFO("SockOptDebug");
        setSocketOption(sock, SockOptDebug, true);
        CHECK(getSocketOption(sock, SockOptDebug));
    }
    #endif

    {
        INFO("SockOptBroadcast");
        std::error_code ec;
        setSocketOption(sock, SockOptBroadcast, true, ec);
        if (ec)
            CHECK(errorEquals(ec, std::errc::no_protocol_option));
        else
            CHECK(getSocketOption(sock, SockOptBroadcast));
    }

    {
        INFO("SockOptReuseAddr");
        setSocketOption(sock, SockOptReuseAddr, true);
        CHECK(getSocketOption(sock, SockOptReuseAddr));
    }

    {
        INFO("SockOptKeepAlive");
        setSocketOption(sock, SockOptKeepAlive, true);
        CHECK(getSocketOption(sock, SockOptKeepAlive));
    }

    {
        INFO("SockOptLinger");
        ::linger linger;
        linger.l_onoff = 1;
        linger.l_linger = 3;
        setSocketOption(sock, SockOptLinger, linger);
        ::linger lingerOut{};
        getSocketOption(sock, SockOptLinger, lingerOut);
        CHECK(lingerOut.l_onoff != 0);
        CHECK(linger.l_linger == lingerOut.l_linger);
    }
}

TEST_CASE( "byte sized socket options" ) {
    {
        auto sock = createSocket(PF_INET, SOCK_DGRAM, 0);
        REQUIRE(sock);

        #ifdef IP_MULTICAST_LOOP
        {
            INFO("SockOptIPv4MulticastLoop");
            setSocketOption(sock, SockOptIPv4MulticastLoop, true);
            CHECK(getSocketOption(sock, SockOptIPv4MulticastLoop) == true);
        }
        #endif
    }
    {
        auto sock = createSocket(PF_INET6, SOCK_DGRAM, 0);
        REQUIRE(sock);

        #ifdef IPV6_MULTICAST_LOOP
        {
            INFO("SockOptIPv6MulticastLoop");
            setSocketOption(sock, SockOptIPv6MulticastLoop, true);
            CHECK(getSocketOption(sock, SockOptIPv6MulticastLoop) == true);
        }
        #endif
    }
}

TEST_CASE( "read-write" ) {

    auto recvSock = createSocket(PF_INET, SOCK_DGRAM, 0);
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    addr.sin_addr.s_addr = htonl(0x7F000001);
    ptl::socklen_t addrLen = sizeof(addr);

    bindSocket(recvSock, reinterpret_cast<sockaddr *>(&addr), addrLen);
    getSocketName(recvSock, reinterpret_cast<sockaddr *>(&addr), &addrLen);
    REQUIRE(addrLen == sizeof(addr));
    REQUIRE(addr.sin_port != 0);
    
    auto sendSock = createSocket(PF_INET, SOCK_DGRAM, 0);

    std::thread sender([&](){
        std::error_code ec;
        auto sent = sendSocket(sendSock, "hello", 6, 0, reinterpret_cast<sockaddr *>(&addr), addrLen, ec);
        CHECK(!ec);
        CHECK(sent == 6);
    });
    
    char buf[6];
    auto received = receiveSocket(recvSock, buf, sizeof(buf), 0);
    CHECK(received == 6);
    CHECK(memcmp(buf, "hello", 6) == 0);
    sender.join();
}

}

#endif 
