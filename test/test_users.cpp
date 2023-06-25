#include <ptl/users.h>

#include <catch2/catch_test_macros.hpp>

#include "common.h"

using namespace ptl;
using namespace std::literals;

TEST_CASE( "user by name" , "[users]") {

    auto res = Passwd::getByName("root");
    REQUIRE(res);
    CHECK(res->pw_uid == 0);
    CHECK(res->pw_name == "root"s);

    res = Passwd::getByName("e$%^&*HHH");
    REQUIRE(!res);

    std::error_code ec;
    res = Passwd::getByName("root", ec);
    REQUIRE(res);
    CHECK(!ec);
    CHECK(res->pw_uid == 0);
    CHECK(res->pw_name == "root"s);

    res = Passwd::getByName("e$%^&*HHH", ec);
    REQUIRE(!res);
    CHECK(!ec);
}

TEST_CASE( "user by id" , "[users]") {

    auto res = Passwd::getById(0);
    REQUIRE(res);
    CHECK(res->pw_uid == 0);
    CHECK(res->pw_name == "root"s);

    res = Passwd::getById(65536);
    REQUIRE(!res);

    std::error_code ec;
    res = Passwd::getById(0, ec);
    REQUIRE(res);
    CHECK(!ec);
    CHECK(res->pw_uid == 0);
    CHECK(res->pw_name == "root"s);

    res = Passwd::getById(65536, ec);
    REQUIRE(!res);
    CHECK(!ec);
}

TEST_CASE( "group by name and id" , "[users]") {

    auto pwres = Passwd::getByName("root");
    REQUIRE(pwres);
    auto gid = pwres->pw_gid;

    auto res = Group::getById(gid);
    REQUIRE(res);
    std::string name = res->gr_name;

    res = Group::getByName(name);
    REQUIRE(res);
    CHECK(res->gr_gid == gid);
    CHECK(res->gr_name == name);

    res = Group::getByName("e$%^&*HHH");
    REQUIRE(!res);

    std::error_code ec;
    res = Group::getByName(name, ec);
    REQUIRE(res);
    CHECK(!ec);
    CHECK(res->gr_gid == gid);
    CHECK(res->gr_name == name);

    res = Group::getByName("e$%^&*HHH", ec);
    REQUIRE(!res);
    CHECK(!ec);

    res = Group::getById(65536);
    REQUIRE(!res);

    res = Group::getById(gid, ec);
    REQUIRE(res);
    CHECK(!ec);
    CHECK(res->gr_gid == gid);
    CHECK(res->gr_name == name);

    res = Group::getById(65536, ec);
    REQUIRE(!res);
    CHECK(!ec);
}
