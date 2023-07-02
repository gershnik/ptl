#ifndef PTL_HEADER_SYSTEM_H_INCLUDED
#define PTL_HEADER_SYSTEM_H_INCLUDED

#include <ptl/core.h>

#include <optional>


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

}


#endif

