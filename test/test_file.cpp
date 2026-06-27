// Copyright (c) 2023, Eugene Gershnik
// SPDX-License-Identifier: BSD-3-Clause

#include <ptl/file.h>


#include "common.h"

#include <cstring>

using namespace ptl;

TEST_SUITE("file_descriptor") {

TEST_CASE( "open file") {

    CHECK_THROWS_MATCHES([]() {
        auto fd = FileDescriptor::open("nonexistant", O_RDONLY);
    }(), std::errc::no_such_file_or_directory);

    std::error_code ec;
    auto fd = FileDescriptor::open("nonexistant", O_RDONLY, ec);
    CHECK(!fd);
    CHECK(errorEquals(ec, std::errc::no_such_file_or_directory));

    Error err;
    fd = FileDescriptor::open("nonexistant", O_RDONLY, err);
    CHECK(!fd);
    CHECK(err == ENOENT);
}

TEST_CASE("FileDescriptor lifecycle") {
    // default ctor
    FileDescriptor empty;
    CHECK(!empty);
    CHECK(empty.get() == -1);

    // move ctor
    auto fd = FileDescriptor::open("test_file", O_WRONLY | O_CREAT, 0644);
    int raw = fd.get();
    CHECK(fd);
    FileDescriptor moved = std::move(fd);
    CHECK(!fd);
    CHECK(moved);
    CHECK(moved.get() == raw);

    // move assignment closes the previous fd
    auto fd2 = FileDescriptor::open("test_file2", O_WRONLY | O_CREAT, 0644);
    fd2 = std::move(moved);
    // (no easy way to check the old fd2 was closed; trust valgrind/asan)

    // detach
    int detached = fd2.detach();
    CHECK(!fd2);
    CHECK(detached >= 0);
    impl::close(detached);

    // close
    auto fd3 = FileDescriptor::open("test_file", O_RDONLY);
    fd3.close();
    CHECK(!fd3);

    std::filesystem::remove("test_file");
    std::filesystem::remove("test_file2");
}

#if PTL_HAVE_MKOSTEMPS
TEST_CASE("openTemp") {
    char tmpl[] = "ptl_test_XXXXXX";
    auto fd = FileDescriptor::openTemp(tmpl);
    CHECK(fd);
    // template was modified to actual filename
    CHECK(std::string_view(tmpl).find("XXXXXX") == std::string_view::npos);
    CHECK(std::filesystem::exists(tmpl));

    // write/read roundtrip
    writeFile(fd, "hello", 5);
    // ...

    std::filesystem::remove(tmpl);

    // with suffix
    char tmpl2[] = "ptl_test_XXXXXX.dat";
    auto fd2 = FileDescriptor::openTemp(tmpl2, 4);
    CHECK(fd2);
    CHECK(std::string_view(tmpl2).ends_with(".dat"));
    std::filesystem::remove(tmpl2);
}
#endif

#ifndef _WIN32

TEST_CASE("chmod") {

    std::error_code ec;
    std::filesystem::remove("test_file", ec);

    {
        auto fd = FileDescriptor::open("test_file", O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
        changeMode(fd, S_IRUSR | S_IWUSR);
        auto status = std::filesystem::status("test_file");
        CHECK(status.permissions() == (std::filesystem::perms::owner_read | std::filesystem::perms::owner_write));
        std::filesystem::remove("test_file");
    }
    {
        auto fd = FileDescriptor::open("test_file", O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
        changeMode(fd, S_IRUSR | S_IWUSR, ec);
        CHECK(!ec);
        auto status = std::filesystem::status("test_file");
        CHECK(status.permissions() == (std::filesystem::perms::owner_read | std::filesystem::perms::owner_write));
        std::filesystem::remove("test_file");
    }
    {
        {auto fd = FileDescriptor::open("test_file", O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);}
        changeMode("test_file", S_IRUSR | S_IWUSR);
        auto status = std::filesystem::status("test_file");
        CHECK(status.permissions() == (std::filesystem::perms::owner_read | std::filesystem::perms::owner_write));
        std::filesystem::remove("test_file");
    }
    {
        {auto fd = FileDescriptor::open("test_file", O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);}
        changeMode("test_file", S_IRUSR | S_IWUSR, ec);
        CHECK(!ec);
        auto status = std::filesystem::status("test_file");
        CHECK(status.permissions() == (std::filesystem::perms::owner_read | std::filesystem::perms::owner_write));
        std::filesystem::remove("test_file");
    }
}

TEST_CASE("mmap") {

    std::error_code ec;
    std::filesystem::remove("test_file", ec);


    {
        auto fd = FileDescriptor::open("test_file", O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
        writeFile(fd, "hello", 5);
    }
    {
        auto fd = FileDescriptor::open("test_file", O_RDONLY);
        struct ::stat st;
        getStatus(fd, st);
        MemoryMap map(nullptr, size_t(st.st_size), PROT_READ, MAP_PRIVATE, fd, 0);
        REQUIRE(map.size() == 5);
        REQUIRE(map.data());
        CHECK(map);
        CHECK(memcmp(map.data(), "hello", 5) == 0);
    }
    {
        auto fd = FileDescriptor::open("test_file", O_RDONLY);
        struct ::stat st;
        getStatus(fd, st);
        MemoryMap map(size_t(st.st_size), PROT_READ, MAP_PRIVATE, fd, 0);
        REQUIRE(map.size() == 5);
        REQUIRE(map.data());
        CHECK(map);
        CHECK(memcmp(map.data(), "hello", 5) == 0);
    }
    {
        auto fd = FileDescriptor::open("test_file", O_RDONLY);
        struct ::stat st;
        getStatus(fd, st);
        MemoryMap map(nullptr, size_t(st.st_size), PROT_READ, MAP_PRIVATE, fd);
        REQUIRE(map.size() == 5);
        REQUIRE(map.data());
        CHECK(map);
        CHECK(memcmp(map.data(), "hello", 5) == 0);
    }
    {
        auto fd = FileDescriptor::open("test_file", O_RDONLY);
        struct ::stat st;
        getStatus(fd, st);
        MemoryMap map(size_t(st.st_size), PROT_READ, MAP_PRIVATE, fd);
        REQUIRE(map.size() == 5);
        REQUIRE(map.data());
        CHECK(map);
        CHECK(memcmp(map.data(), "hello", 5) == 0);
    }
    {
        auto fd = FileDescriptor::open("test_file", O_RDONLY);
        struct ::stat st;
        getStatus(fd, st);
        MemoryMap map(nullptr, size_t(st.st_size), PROT_READ, MAP_PRIVATE, fd, 0, ec);
        CHECK(!ec);
        REQUIRE(map.size() == 5);
        REQUIRE(map.data());
        CHECK(map);
        CHECK(memcmp(map.data(), "hello", 5) == 0);
    }

}

TEST_CASE("FILE * as file-like") {
    FILE * fp = std::fopen("test_file", "w");
    REQUIRE(fp);
    changeMode(fp, S_IRUSR | S_IWUSR);   // uses FileDescriptorTraits<FILE*>
    std::fclose(fp);
    // verify mode
    std::filesystem::remove("test_file");
}

TEST_CASE("makeDirectory and changeDirectory") {
    auto originalCwd = std::filesystem::current_path();
    std::filesystem::remove_all("ptl_test_dir");
    
    makeDirectory("ptl_test_dir", S_IRWXU);
    CHECK(std::filesystem::is_directory("ptl_test_dir"));

    changeDirectory("ptl_test_dir");
    CHECK(std::filesystem::current_path().filename() == "ptl_test_dir");
    
    changeDirectory(originalCwd);   // restore

    // by fd
    auto dirFd = FileDescriptor::open("ptl_test_dir", O_RDONLY | O_DIRECTORY);
    changeDirectory(dirFd);
    CHECK(std::filesystem::current_path().filename() == "ptl_test_dir");
    changeDirectory(originalCwd);

    std::filesystem::remove_all("ptl_test_dir");
}

#if PTL_HAVE_MKDIRAT
TEST_CASE("makeDirectoryAt") {
    auto cwdFd = FileDescriptor::open(".", O_RDONLY | O_DIRECTORY);
    makeDirectoryAt(cwdFd, "ptl_test_subdir", S_IRWXU);
    CHECK(std::filesystem::is_directory("ptl_test_subdir"));
    std::filesystem::remove_all("ptl_test_subdir");
}
#endif

#if !defined(__EMSCRIPTEN__)
TEST_CASE("flock") {
    auto fd = FileDescriptor::open("test_file", O_WRONLY | O_CREAT, 0644);

    lockFile(fd, FileLock::Exclusive);
    
    // try-lock from another fd to the same file fails
    auto fd2 = FileDescriptor::open("test_file", O_WRONLY);
    CHECK(tryLockFile(fd2, FileLock::Exclusive) == false);

    unlockFile(fd);
    CHECK(tryLockFile(fd2, FileLock::Exclusive) == true);
    unlockFile(fd2);
    
    std::filesystem::remove("test_file");
}
#endif

#endif

}
