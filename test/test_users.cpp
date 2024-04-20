// Copyright (c) 2023, Eugene Gershnik
// SPDX-License-Identifier: BSD-3-Clause

#include <ptl/users.h>

#include "common.h"

using namespace ptl;
using namespace std::literals;

#ifndef _WIN32

struct UserInfo {
    std::string user;
    uid_t uid;
    gid_t gid;
};

static auto getMyself() -> const UserInfo & {
    static UserInfo theInfo = []() {
        return UserInfo {
            .user = shell({ "whoami" }),
            .uid = uid_t(atoi(shell({ "id", "-u" }).c_str())),
            .gid = gid_t(atoi(shell({ "id", "-g" }).c_str()))
        };
    }();


    return theInfo;
}

TEST_SUITE("users") {

TEST_CASE( "user by name" ) {

    auto & myself = getMyself();

    auto res = Passwd::getByName(myself.user);
    REQUIRE(res);
    CHECK(res->pw_uid == myself.uid);
    CHECK(res->pw_name == myself.user);

    res = Passwd::getByName("e$%^&*HHH");
    REQUIRE(!res);

    std::error_code ec;
    res = Passwd::getByName(myself.user, ec);
    REQUIRE(res);
    CHECK(!ec);
    CHECK(res->pw_uid == myself.uid);
    CHECK(res->pw_name == myself.user);

    res = Passwd::getByName("e$%^&*HHH", ec);
    REQUIRE(!res);
    CHECK(!ec);
}

TEST_CASE( "user by id" ) {

    auto & myself = getMyself();

    auto res = Passwd::getById(myself.uid);
    REQUIRE(res);
    CHECK(res->pw_uid == myself.uid);
    CHECK(res->pw_name == myself.user);

    res = Passwd::getById(65536);
    REQUIRE(!res);

    std::error_code ec;
    res = Passwd::getById(myself.uid, ec);
    REQUIRE(res);
    CHECK(!ec);
    CHECK(res->pw_uid == myself.uid);
    CHECK(res->pw_name == myself.user);

    res = Passwd::getById(65536, ec);
    REQUIRE(!res);
    CHECK(!ec);
}

#if !defined(__ANDROID__) || (defined(__ANDROID__) && __ANDROID_API__ >= 24)

TEST_CASE( "group by name and id" ) {

    auto & myself = getMyself();

    auto res = Group::getById(myself.gid);
    REQUIRE(res);
    std::string name = res->gr_name;

    res = Group::getByName(name);
    REQUIRE(res);
    CHECK(res->gr_gid == myself.gid);
    CHECK(res->gr_name == name);

    res = Group::getByName("e$%^&*HHH");
    REQUIRE(!res);

    std::error_code ec;
    res = Group::getByName(name, ec);
    REQUIRE(res);
    CHECK(!ec);
    CHECK(res->gr_gid == myself.gid);
    CHECK(res->gr_name == name);

    res = Group::getByName("e$%^&*HHH", ec);
    REQUIRE(!res);
    CHECK(!ec);

    res = Group::getById(65536);
    REQUIRE(!res);

    res = Group::getById(myself.gid, ec);
    REQUIRE(res);
    CHECK(!ec);
    CHECK(res->gr_gid == myself.gid);
    CHECK(res->gr_name == name);

    res = Group::getById(65536, ec);
    REQUIRE(!res);
    CHECK(!ec);
}

#endif //ANDROID check

}

#endif
