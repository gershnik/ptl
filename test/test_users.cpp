#include <ptl/users.h>
#include <ptl/spawn.h>

#include <catch2/catch_test_macros.hpp>

#include "common.h"

using namespace ptl;
using namespace std::literals;

#ifndef _WIN32

struct UserInfo {
    std::string user;
    uid_t uid;
    gid_t gid;
};

static auto getMyself() -> UserInfo {
    UserInfo ret;

    {
        auto [readPipe, writePipe] = FileDescriptor::pipe();
        SpawnFileActions fa;
        fa.addDup2(writePipe, stdout);
        auto child = spawn({ "whoami" }, SpawnSettings().fileActions(fa).usePath());
        writePipe.close();
        ret.user = rtrim(readAll(readPipe));
        child.wait();
    }
    {
        auto [readPipe, writePipe] = FileDescriptor::pipe();
        SpawnFileActions fa;
        fa.addDup2(writePipe, stdout);
        auto child = spawn({ "id", "-u" }, SpawnSettings().fileActions(fa).usePath());
        writePipe.close();
        auto uidstr = rtrim(readAll(readPipe));
        ret.uid = atoi(uidstr.c_str());
        child.wait();
    }
    {
        auto [readPipe, writePipe] = FileDescriptor::pipe();
        SpawnFileActions fa;
        fa.addDup2(writePipe, stdout);
        auto child = spawn({ "id", "-g" }, SpawnSettings().fileActions(fa).usePath());
        writePipe.close();
        auto gidstr = rtrim(readAll(readPipe));
        ret.gid = atoi(gidstr.c_str());
        child.wait();
    }


    return ret;
}

static auto g_userInfo = getMyself();

TEST_CASE( "user by name" , "[users]") {

    auto res = Passwd::getByName(g_userInfo.user);
    REQUIRE(res);
    CHECK(res->pw_uid == g_userInfo.uid);
    CHECK(res->pw_name == g_userInfo.user);

    res = Passwd::getByName("e$%^&*HHH");
    REQUIRE(!res);

    std::error_code ec;
    res = Passwd::getByName(g_userInfo.user, ec);
    REQUIRE(res);
    CHECK(!ec);
    CHECK(res->pw_uid == g_userInfo.uid);
    CHECK(res->pw_name == g_userInfo.user);

    res = Passwd::getByName("e$%^&*HHH", ec);
    REQUIRE(!res);
    CHECK(!ec);
}

TEST_CASE( "user by id" , "[users]") {

    auto res = Passwd::getById(g_userInfo.uid);
    REQUIRE(res);
    CHECK(res->pw_uid == g_userInfo.uid);
    CHECK(res->pw_name == g_userInfo.user);

    res = Passwd::getById(65536);
    REQUIRE(!res);

    std::error_code ec;
    res = Passwd::getById(g_userInfo.uid, ec);
    REQUIRE(res);
    CHECK(!ec);
    CHECK(res->pw_uid == g_userInfo.uid);
    CHECK(res->pw_name == g_userInfo.user);

    res = Passwd::getById(65536, ec);
    REQUIRE(!res);
    CHECK(!ec);
}

TEST_CASE( "group by name and id" , "[users]") {

    auto res = Group::getById(g_userInfo.gid);
    REQUIRE(res);
    std::string name = res->gr_name;

    res = Group::getByName(name);
    REQUIRE(res);
    CHECK(res->gr_gid == g_userInfo.gid);
    CHECK(res->gr_name == name);

    res = Group::getByName("e$%^&*HHH");
    REQUIRE(!res);

    std::error_code ec;
    res = Group::getByName(name, ec);
    REQUIRE(res);
    CHECK(!ec);
    CHECK(res->gr_gid == g_userInfo.gid);
    CHECK(res->gr_name == name);

    res = Group::getByName("e$%^&*HHH", ec);
    REQUIRE(!res);
    CHECK(!ec);

    res = Group::getById(65536);
    REQUIRE(!res);

    res = Group::getById(g_userInfo.gid, ec);
    REQUIRE(res);
    CHECK(!ec);
    CHECK(res->gr_gid == g_userInfo.gid);
    CHECK(res->gr_name == name);

    res = Group::getById(65536, ec);
    REQUIRE(!res);
    CHECK(!ec);
}

#endif
