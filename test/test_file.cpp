// Copyright (c) 2023, Eugene Gershnik
// SPDX-License-Identifier: BSD-3-Clause

#include <ptl/file.h>

#include <catch2/catch_test_macros.hpp>

#include "common.h"

using namespace ptl;

TEST_CASE( "open file" , "[file_descriptor]") {

    CHECK_THROWS_MATCHES([]() {
        auto fd = FileDescriptor::open("nonexistant", O_RDONLY);
    }(), std::system_error, EqualsSystemError(std::errc::no_such_file_or_directory));

    std::error_code ec;
    auto fd = FileDescriptor::open("nonexistant", O_RDONLY, ec);
    CHECK(!fd);
    CHECK(ec.value() == ENOENT);
}

#ifndef _WIN32

TEST_CASE("chmod", "[file_descriptor]") {

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

#endif

