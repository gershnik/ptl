// Copyright (c) 2023, Eugene Gershnik
// SPDX-License-Identifier: BSD-3-Clause

#ifndef HEADER_COMMON_H_INCLUDED
#define HEADER_COMMON_H_INCLUDED

#include <ptl/errors.h>

#include <doctest.h>

#define CHECK_THROWS_MATCHES(expr, err_code) \
    try { \
        expr; \
        FAIL("expected exception not thrown"); \
    } catch(std::system_error & ex) { \
        CHECK(ptl::errorEquals(ex.code(), (err_code))); \
    } catch (...) { \
        FAIL("exception thrown of a wrong type"); \
    } 


auto readAll(const auto & readable) -> std::string {
    std::string res;
    while(true) {
        auto offset = res.size();
        constexpr size_t chunk = 5;
        res.resize(offset + chunk);
        auto readCount = readFile(readable, res.data() + offset, chunk);
        res.resize(offset + readCount);
        if (readCount == 0)
            break;
    }
    return res;
}

inline auto rtrim(const std::string & str) -> std::string {
    auto end = std::find_if(str.rbegin(), str.rend(), [](char c) {
        return !isspace(c);
    });
    return std::string(str.begin(), end.base());
}

std::string shell(const ptl::StringRefArray & args);

#endif