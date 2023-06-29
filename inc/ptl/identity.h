#ifndef PTL_HEADER_IDENTITY_H_INCLUDED
#define PTL_HEADER_IDENTITY_H_INCLUDED

#include <ptl/core.h>

#include <utility>

#ifndef _WIN32

namespace ptl::inline v0 {

    void setUid(uid_t uid, PTL_ERROR_REF_ARG(err)) noexcept(PTL_ERROR_NOEXCEPT(err))
    requires(PTL_ERROR_REQ(err)) {
        clearError(PTL_ERROR_REF(err));
        if (setuid(uid) != 0) {
            handleError(PTL_ERROR_REF(err), errno, "setuid({}) failed", uid);
        }
    }

    void setGid(gid_t gid, PTL_ERROR_REF_ARG(err)) noexcept(PTL_ERROR_NOEXCEPT(err))
    requires(PTL_ERROR_REQ(err)) {
        clearError(PTL_ERROR_REF(err));
        if (setgid(gid) != 0) {
            handleError(PTL_ERROR_REF(err), errno, "setgid({}) failed", gid);
        }
    }

    void setEffectiveUid(uid_t uid, PTL_ERROR_REF_ARG(err)) noexcept(PTL_ERROR_NOEXCEPT(err))
    requires(PTL_ERROR_REQ(err)) {
        clearError(PTL_ERROR_REF(err));
        if (seteuid(uid) != 0) {
            handleError(PTL_ERROR_REF(err), errno, "seteuid({}) failed", uid);
        }
    }

    void setEffectiveGid(gid_t gid, PTL_ERROR_REF_ARG(err)) noexcept(PTL_ERROR_NOEXCEPT(err))
    requires(PTL_ERROR_REQ(err)) {
        clearError(PTL_ERROR_REF(err));
        if (setegid(gid) != 0) {
            handleError(PTL_ERROR_REF(err), errno, "setegid({}) failed", gid);
        }
    }

}

#endif

#endif 
