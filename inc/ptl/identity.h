// Copyright (c) 2023, Eugene Gershnik
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PTL_HEADER_IDENTITY_H_INCLUDED
#define PTL_HEADER_IDENTITY_H_INCLUDED

#include <ptl/core.h>

#include <utility>

#ifndef _WIN32

namespace ptl::inline v0 {

    void setUid(uid_t uid, PTL_ERROR_REF_ARG(err))
    requires(PTL_ERROR_REQ(err)) {
        if (setuid(uid) != 0)
            handleError(PTL_ERROR_REF(err), errno, "setuid({}) failed", uid);
        else
            clearError(PTL_ERROR_REF(err));
    }

    void setGid(gid_t gid, PTL_ERROR_REF_ARG(err))
    requires(PTL_ERROR_REQ(err)) {
        if (setgid(gid) != 0) 
            handleError(PTL_ERROR_REF(err), errno, "setgid({}) failed", gid);
        else
            clearError(PTL_ERROR_REF(err));
    }

    void setEffectiveUid(uid_t uid, PTL_ERROR_REF_ARG(err))
    requires(PTL_ERROR_REQ(err)) {
        if (seteuid(uid) != 0)
            handleError(PTL_ERROR_REF(err), errno, "seteuid({}) failed", uid);
        else
            clearError(PTL_ERROR_REF(err));
    }

    void setEffectiveGid(gid_t gid, PTL_ERROR_REF_ARG(err))
    requires(PTL_ERROR_REQ(err)) {
        if (setegid(gid) != 0) 
            handleError(PTL_ERROR_REF(err), errno, "setegid({}) failed", gid);
        else
            clearError(PTL_ERROR_REF(err));
    }

}

#endif

#endif 
