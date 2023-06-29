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

static auto getMyself() -> const UserInfo & {
    static UserInfo theInfo = []() {
        UserInfo ret;
        {
            auto [readPipe, writePipe] = Pipe::create();
            SpawnFileActions fa;
            fa.addDup2(writePipe, stdout);
            auto child = spawn({ "whoami" }, SpawnSettings().fileActions(fa).usePath());
            writePipe.close();
            ret.user = rtrim(readAll(readPipe));
            child.wait();
        }
        {
            auto [readPipe, writePipe] = Pipe::create();
            SpawnFileActions fa;
            fa.addDup2(writePipe, stdout);
            auto child = spawn({ "id", "-u" }, SpawnSettings().fileActions(fa).usePath());
            writePipe.close();
            auto uidstr = rtrim(readAll(readPipe));
            ret.uid = atoi(uidstr.c_str());
            child.wait();
        }
        {
            auto [readPipe, writePipe] = Pipe::create();
            SpawnFileActions fa;
            fa.addDup2(writePipe, stdout);
            auto child = spawn({ "id", "-g" }, SpawnSettings().fileActions(fa).usePath());
            writePipe.close();
            auto gidstr = rtrim(readAll(readPipe));
            ret.gid = atoi(gidstr.c_str());
            child.wait();
        }
        return ret;
    }();


    return theInfo;
}

TEST_CASE( "user by name" , "[users]") {

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

TEST_CASE( "user by id" , "[users]") {

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

TEST_CASE( "group by name and id" , "[users]") {

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

#endif
