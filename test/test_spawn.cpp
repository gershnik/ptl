// Copyright (c) 2023, Eugene Gershnik
// SPDX-License-Identifier: BSD-3-Clause

#include <ptl/spawn.h>
#include <ptl/signal.h>

#include "common.h"

#include <array>

using namespace ptl;

#if !defined(__EMSCRIPTEN__)

TEST_SUITE("spawn") {

#if !defined(__ANDROID__) || (defined(__ANDROID__) && __ANDROID_API__ >= 28)

TEST_CASE( "spawn signatures" ) {

    const char * dummyArgs[] = {
       "abc"
    };

    static_assert(std::is_same_v<ChildProcess, decltype(spawn("abc", dummyArgs, SpawnSettings()))>);
    static_assert(std::is_same_v<ChildProcess, decltype(spawn("abc", dummyArgs))>);
    static_assert(std::is_same_v<ChildProcess, decltype(spawn(dummyArgs))>);
    static_assert(std::is_same_v<ChildProcess, decltype(spawn(dummyArgs, SpawnSettings()))>);

    std::vector<std::string> dummyVec = {"abc"};

    static_assert(std::is_same_v<ChildProcess, decltype(spawn("abc", dummyVec, SpawnSettings()))>);
    static_assert(std::is_same_v<ChildProcess, decltype(spawn("abc", dummyVec))>);
    static_assert(std::is_same_v<ChildProcess, decltype(spawn(dummyVec))>);
    static_assert(std::is_same_v<ChildProcess, decltype(spawn(dummyVec, SpawnSettings()))>);

    std::array dummyArr = {"abc"};

    static_assert(std::is_same_v<ChildProcess, decltype(spawn("abc", dummyArr, SpawnSettings()))>);
    static_assert(std::is_same_v<ChildProcess, decltype(spawn("abc", dummyArr))>);
    static_assert(std::is_same_v<ChildProcess, decltype(spawn(dummyArr))>);
    static_assert(std::is_same_v<ChildProcess, decltype(spawn(dummyArr, SpawnSettings()))>);

    static_assert(std::is_same_v<ChildProcess, decltype(spawn("abc", {"xyz"}, SpawnSettings()))>);
    static_assert(std::is_same_v<ChildProcess, decltype(spawn("abc", {"xyz"}))>);
    static_assert(std::is_same_v<ChildProcess, decltype(spawn({"xyz"}))>);
    static_assert(std::is_same_v<ChildProcess, decltype(spawn({"xyz"}, SpawnSettings()))>);

    static_assert(std::is_same_v<ChildProcess, decltype(spawn("abc", {"xyz"}, {"env1"}, SpawnSettings()))>);
    static_assert(std::is_same_v<ChildProcess, decltype(spawn("abc", {"xyz"}, {"env1"}))>);
    static_assert(std::is_same_v<ChildProcess, decltype(spawn({"xyz"}, {"env1"}))>);
    static_assert(std::is_same_v<ChildProcess, decltype(spawn({"xyz"}, {"env1"}, SpawnSettings()))>);

}

#endif

#ifndef _WIN32

#if !defined(__ANDROID__) || (defined(__ANDROID__) && __ANDROID_API__ >= 28)

TEST_CASE( "spawn" ) {

    //Android and OpenBSD don't error on invalid executable,
    //they simply fork/execs so you can only wait on child and "see"
    #if !defined(__ANDROID__) && !defined(__OpenBSD__)

    //QEMU user emulation also doesn't check path existence
    if (getenv("QEMU_LD_PREFIX") == nullptr) {
        printf("BLUB\n");

        auto launch = [](std::filesystem::path exe, auto && ...args) {
            SpawnFileActions act;
            act.addOpen(stdout, "/dev/null", O_WRONLY, 0);
            SpawnAttr spawnAttr;
            spawnAttr.setFlags(POSIX_SPAWN_SETSIGDEF);
            #ifndef __CYGWIN__
                spawnAttr.setSigDefault(SignalSet::all());
            #else
                auto sigs = SignalSet::all();
                sigs.del(SIGKILL);
                sigs.del(SIGSTOP);
                spawnAttr.setSigDefault(sigs);
            #endif

            auto proc = spawn({exe.c_str(), args...}, SpawnSettings().attr(spawnAttr).fileActions(act));
        };

        auto launchEc = [](std::error_code & ec, std::filesystem::path exe, auto && ...args) {
            auto proc = spawn({exe.c_str(), args...}, ec);
        };

        CHECK_THROWS_MATCHES(launch("no_such_exe", "-c", "ls"), std::errc::no_such_file_or_directory);
        CHECK_NOTHROW(launch("/bin/sh", "-c", "ls"));
        std::error_code ec;
        CHECK_NOTHROW(launchEc(ec, "no_such_exe", "-c", "ls"));
        CHECK(errorEquals(ec, std::errc::no_such_file_or_directory));
        CHECK_NOTHROW(launchEc(ec, "no_such_exe", "-c", "ls", (const char *)nullptr));
        CHECK(errorEquals(ec, std::errc::no_such_file_or_directory));

    }

    #endif

    {
        auto [read, write] = Pipe::create();

        SpawnFileActions act;
        act.addDuplicateTo(write, stdout);
        auto proc = spawn({"sh", "-c", "echo $PTL_STRING"}, {"PTL_STRING=haha"}, SpawnSettings().fileActions(act).usePath());
        write.close();

        std::string res;
        while(true) {
            auto offset = res.size();
            constexpr size_t chunk = 5;
            res.resize(offset + chunk);
            auto readCount = readFile(read, res.data() + offset, chunk);
            res.resize(offset + readCount);
            if (readCount == 0)
                break;
        }
        auto stat = proc.wait().value();
        CHECK(WIFEXITED(stat));
        CHECK(WEXITSTATUS(stat) == 0);
        CHECK(res == "haha\n");
    }
}

#endif //ANDROID version check


TEST_CASE( "fork_exec" ) {

    auto [read, write] = Pipe::create();

    auto childProc = forkProcess();
    if (childProc) {
        write.close();

        std::string res;
        while(true) {
            auto offset = res.size();
            constexpr size_t chunk = 5;
            res.resize(offset + chunk);
            auto readCount = readFile(read, res.data() + offset, chunk);
            res.resize(offset + readCount);
            if (readCount == 0)
                break;
        }
        auto stat = childProc.wait().value();
        CHECK(WIFEXITED(stat));
        CHECK(WEXITSTATUS(stat) == 0);
        CHECK(res == "hoho\n");

    } else {
        
        read.close();
        duplicateTo(write, stdout);
        execp({"sh", "-c", "echo hoho"});
    }
}

#else

TEST_CASE( "spawn" ) {

    auto launch = [](std::filesystem::path exe, auto && ...args) {
        auto proc = spawn({exe.string().c_str(), args...});
    };

    auto launchEc = [](std::error_code & ec, std::filesystem::path exe, auto && ...args) {
        auto proc = spawn({exe.string().c_str(), args...}, ec);
    };

    CHECK_THROWS_MATCHES(launch("no_such_exe", "/c", "dir"), std::errc::no_such_file_or_directory);
    CHECK_NOTHROW(launch("C:\\Windows\\System32\\cmd.exe", "/c", "dir >nul:"));
    std::error_code ec;
    CHECK_NOTHROW(launchEc(ec, "no_such_exe", "/c", "dir"));
    CHECK(errorEquals(ec, std::errc::no_such_file_or_directory));
    CHECK_NOTHROW(launchEc(ec, "no_such_exe", "/c", "dir", (const char *)nullptr));
    CHECK(errorEquals(ec, std::errc::no_such_file_or_directory));

    {
        auto [read, write] = Pipe::create();

        auto proc = spawn({"cmd", "/c", ">&4 echo %PTL_STRING%"}, {"PTL_STRING=haha"}, SpawnSettings().usePath());
        write.close();

        std::string res;
        while(true) {
            auto offset = res.size();
            constexpr size_t chunk = 5;
            res.resize(offset + chunk);
            auto readCount = readFile(read, res.data() + offset, chunk);
            res.resize(offset + readCount);
            if (readCount == 0)
                break;
        }
        auto stat = proc.wait().value();
        CHECK(stat == 0);
        CHECK(res == "haha\r\n");
    }
}

#endif

}

#endif
