#ifndef HEADER_COMMON_H_INCLUDED
#define HEADER_COMMON_H_INCLUDED

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

#endif