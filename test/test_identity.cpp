#include <ptl/identity.h>
//#include <ptl/spawn.h>

#include <catch2/catch_test_macros.hpp>

#include "common.h"

using namespace ptl;

TEST_CASE( "basics" , "[identity]") {

    auto mine = Identity::real();
    CHECK(mine.uid == getuid());
    CHECK(mine.gid == getgid());

    auto effective = Identity::effective();
    CHECK(effective.uid == geteuid());
    CHECK(effective.gid == getegid());

}