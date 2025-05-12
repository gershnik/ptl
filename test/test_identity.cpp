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

    if (getuid() == 0) {

        auto pipe = Pipe::create();

        auto child = forkProcess();
        if (!child) {
            pipe.readEnd.close();
            setGid(1);
            setUid(1);
            auto newGid = getgid();
            auto newUid = getuid();
            writeFile(pipe.writeEnd, &newGid, sizeof(newGid));
            writeFile(pipe.writeEnd, &newUid, sizeof(newUid));
            exit(0);
        }
        pipe.writeEnd.close();
        gid_t gid;
        uid_t uid;
        readFile(pipe.readEnd, &gid, sizeof(gid));
        readFile(pipe.readEnd, &uid, sizeof(uid));
        CHECK(gid == 1);
        CHECK(uid == 1);
        child.wait();
    }
}

TEST_CASE( "supplemental") {

    auto groups = getGroups();
    CHECK(!groups.empty());
    
    if (getuid() == 0) {

        auto pipe = Pipe::create();

        auto child = forkProcess();
        if (!child) {
            pipe.readEnd.close();
            setGroups({1});
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
