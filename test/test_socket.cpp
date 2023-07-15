// Copyright (c) 2023, Eugene Gershnik
// SPDX-License-Identifier: BSD-3-Clause

#include <ptl/socket.h>

#include <catch2/catch_test_macros.hpp>

#include "common.h"

using namespace ptl;

TEST_CASE( "socket creation" , "[signal]") {

    auto sock = createSocket(PF_INET, SOCK_STREAM, 0);
}