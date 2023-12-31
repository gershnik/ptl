// Copyright (c) 2023, Eugene Gershnik
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PTL_HEADER_SYSTEM_H_INCLUDED
#define PTL_HEADER_SYSTEM_H_INCLUDED

#include <ptl/core.h>

#include <optional>
#include <span>

#ifndef _WIN32

namespace ptl::inline v0 {

    inline auto systemConfig(int name) -> std::optional<long>{
        errno = 0;
        auto ret = ::sysconf(name);
        if (ret == -1) {
            if (int err = errno; err != 0) {
                throwErrorCode(err, "sysconf({}) failed", name);
            }
            return std::nullopt;
        }
        return ret;
    }

    inline void getHostName(std::span<char> buf, PTL_ERROR_REF_ARG(err)) 
    requires(PTL_ERROR_REQ(err)) {
        if (::gethostname(buf.data(), buf.size()) != 0) {
            handleError(PTL_ERROR_REF(err), errno, "gethostname(...,{}) failed", buf.size());
        } else {
            clearError(PTL_ERROR_REF(err));
            if (auto size = buf.size())
                buf[size - 1] = '\0';
        }
    }

}

#endif


#endif

