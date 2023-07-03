#ifndef HEADER_COMMON_H_INCLUDED
#define HEADER_COMMON_H_INCLUDED

#include <ptl/core.h>

#include <catch2/matchers/catch_matchers_templated.hpp>

struct SystemErrorMatcher : Catch::Matchers::MatcherGenericBase {
    SystemErrorMatcher(std::errc code): m_code(std::make_error_code(code))
    {}

    bool match(const std::system_error & ex) const {
        return ex.code() == m_code;
    }

    std::string describe() const override {
        return "Equals: " + m_code.message();
    }

private:
    std::error_code m_code;
};

inline auto EqualsSystemError(std::errc code) -> SystemErrorMatcher {
    return SystemErrorMatcher(code);
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