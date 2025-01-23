// Copyright (c) 2023, Eugene Gershnik
// SPDX-License-Identifier: BSD-3-Clause

#include <ptl/identity.h>
#include <ptl/spawn.h>
#include <ptl/file.h>

#include "common.h"

using namespace ptl;

#if !defined(_WIN32) && !defined(__EMSCRIPTEN__)

TEST_SUITE("identity") {

TEST_CASE( "basics") {

    //Hmmm, how to test it...
}

TEST_CASE( "supplemental") {

    auto groups = getGroups();
    CHECK(!groups.empty());
    
    if (getuid() == 0) {

        auto pipe = Pipe::create();

        auto child = forkProcess();
        if (!child) {
            pipe.readEnd.close();
            setGroups({});
            setGid(1);
            setUid(1);
            auto newGroups = getGroups();
            size_t count = newGroups.size();
            writeFile(pipe.writeEnd, &count, sizeof(count));
            for(auto id: newGroups) {
                writeFile(pipe.writeEnd, &id, sizeof(id));
            }
            exit(0);
        }
        pipe.writeEnd.close();
        size_t count;
        readFile(pipe.readEnd, &count, sizeof(count));
        CHECK(count == 1);
        if (count == 1) {
            gid_t gid;
            readFile(pipe.readEnd, &gid, sizeof(gid));
            CHECK(gid == 1);
        }
        child.wait();
        
    }
}

}

#endif
