#include <ptl/spawn.h>
#include <ptl/signal.h>

#include <catch2/catch_test_macros.hpp>

#include "common.h"

using namespace ptl;

TEST_CASE( "spawn signatures" , "[spawn]") {

    const char * dummyArgs[] = {
       "abc"
    };

    STATIC_CHECK(std::is_same_v<ChildProcess, decltype(spawn("abc", dummyArgs, SpawnSettings()))>);
    STATIC_CHECK(std::is_same_v<ChildProcess, decltype(spawn("abc", dummyArgs))>);
    STATIC_CHECK(std::is_same_v<ChildProcess, decltype(spawn(dummyArgs))>);
    STATIC_CHECK(std::is_same_v<ChildProcess, decltype(spawn(dummyArgs, SpawnSettings()))>);

    std::vector<std::string> dummyVec = {"abc"};

    STATIC_CHECK(std::is_same_v<ChildProcess, decltype(spawn("abc", dummyVec, SpawnSettings()))>);
    STATIC_CHECK(std::is_same_v<ChildProcess, decltype(spawn("abc", dummyVec))>);
    STATIC_CHECK(std::is_same_v<ChildProcess, decltype(spawn(dummyVec))>);
    STATIC_CHECK(std::is_same_v<ChildProcess, decltype(spawn(dummyVec, SpawnSettings()))>);

    std::array dummyArr = {"abc"};

    STATIC_CHECK(std::is_same_v<ChildProcess, decltype(spawn("abc", dummyArr, SpawnSettings()))>);
    STATIC_CHECK(std::is_same_v<ChildProcess, decltype(spawn("abc", dummyArr))>);
    STATIC_CHECK(std::is_same_v<ChildProcess, decltype(spawn(dummyArr))>);
    STATIC_CHECK(std::is_same_v<ChildProcess, decltype(spawn(dummyArr, SpawnSettings()))>);

    STATIC_CHECK(std::is_same_v<ChildProcess, decltype(spawn("abc", {"xyz"}, SpawnSettings()))>);
    STATIC_CHECK(std::is_same_v<ChildProcess, decltype(spawn("abc", {"xyz"}))>);
    STATIC_CHECK(std::is_same_v<ChildProcess, decltype(spawn({"xyz"}))>);
    STATIC_CHECK(std::is_same_v<ChildProcess, decltype(spawn({"xyz"}, SpawnSettings()))>);

    STATIC_CHECK(std::is_same_v<ChildProcess, decltype(spawn("abc", {"xyz"}, {"env1"}, SpawnSettings()))>);
    STATIC_CHECK(std::is_same_v<ChildProcess, decltype(spawn("abc", {"xyz"}, {"env1"}))>);
    STATIC_CHECK(std::is_same_v<ChildProcess, decltype(spawn({"xyz"}, {"env1"}))>);
    STATIC_CHECK(std::is_same_v<ChildProcess, decltype(spawn({"xyz"}, {"env1"}, SpawnSettings()))>);

}

#ifndef _WIN32
TEST_CASE( "spawn" , "[spawn]") {

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

    CHECK_THROWS_MATCHES(launch("no_such_exe", "-c", "ls"), std::system_error, EqualsSystemError(std::errc::no_such_file_or_directory));
    CHECK_NOTHROW(launch("/bin/sh", "-c", "ls"));
    std::error_code ec;
    CHECK_NOTHROW(launchEc(ec, "no_such_exe", "-c", "ls"));
    CHECK(ec.value() == ENOENT);
    CHECK_NOTHROW(launchEc(ec, "no_such_exe", "-c", "ls", (const char *)nullptr));
    CHECK(ec.value() == ENOENT);

    {
        auto [read, write] = FileDescriptor::pipe();

        SpawnFileActions act;
        act.addDup2(write, stdout);
        auto proc = spawn({"sh", "-c", "echo $PTL_STRING"}, {"PTL_STRING=haha"}, SpawnSettings().fileActions(act).usePath());
        write.close();

        std::string res;
        while(true) {
            auto offset = res.size();
            constexpr size_t chunk = 5;
            res.resize(offset + chunk);
            auto readCount = read.read(res.data() + offset, chunk);
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


TEST_CASE( "fork_exec" , "[spawn]") {

    auto [read, write] = FileDescriptor::pipe();

    auto childProc = forkProcess();
    if (childProc) {
        write.close();

        std::string res;
        while(true) {
            auto offset = res.size();
            constexpr size_t chunk = 5;
            res.resize(offset + chunk);
            auto readCount = read.read(res.data() + offset, chunk);
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
        write.dup2(stdout);
        execp({"sh", "-c", "echo hoho"});
    }
}

#else

TEST_CASE( "spawn" , "[spawn]") {

    auto launch = [](std::filesystem::path exe, auto && ...args) {
        auto proc = spawn({exe.string().c_str(), args...});
    };

    auto launchEc = [](std::error_code & ec, std::filesystem::path exe, auto && ...args) {
        auto proc = spawn({exe.string().c_str(), args...}, ec);
    };

    CHECK_THROWS_MATCHES(launch("no_such_exe", "/c", "dir"), std::system_error, EqualsSystemError(std::errc::no_such_file_or_directory));
    CHECK_NOTHROW(launch("C:\\Windows\\System32\\cmd.exe", "/c", "dir >nul:"));
    std::error_code ec;
    CHECK_NOTHROW(launchEc(ec, "no_such_exe", "/c", "dir"));
    CHECK(ec.value() == ENOENT);
    CHECK_NOTHROW(launchEc(ec, "no_such_exe", "/c", "dir", (const char *)nullptr));
    CHECK(ec.value() == ENOENT);

    {
        auto [read, write] = FileDescriptor::pipe();

        auto proc = spawn({"cmd", "/c", ">&4 echo %PTL_STRING%"}, {"PTL_STRING=haha"}, SpawnSettings().usePath());
        write.close();

        std::string res;
        while(true) {
            auto offset = res.size();
            constexpr size_t chunk = 5;
            res.resize(offset + chunk);
            auto readCount = read.read(res.data() + offset, chunk);
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