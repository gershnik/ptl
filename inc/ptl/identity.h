// Copyright (c) 2023, Eugene Gershnik
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PTL_HEADER_IDENTITY_H_INCLUDED
#define PTL_HEADER_IDENTITY_H_INCLUDED

#include <ptl/core.h>

#include <utility>
#include <span>

#ifndef _WIN32

#include <grp.h>

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
    
    auto getGroups(PTL_ERROR_REF_ARG(err)) -> std::vector<gid_t>
    requires(PTL_ERROR_REQ(err)) {
        std::vector<gid_t> ret;
        for ( ; ; ) {
            int res = ::getgroups(0, nullptr);
            if (res < 0) {
                handleError(PTL_ERROR_REF(err), errno, "getgroups(0, NULL) failed");
                break;
            } 
            if (res > 0) {
                ret.resize(res);
                res = ::getgroups(int(ret.size()), ret.data());
                if (res < 0) {
                    int code = errno;
                    if (code == EINVAL) {//avoiding TOCTOU
                        ret.clear();
                        continue;
                    }
                    handleError(PTL_ERROR_REF(err), code, "getgroups({}) failed", int(ret.size()));
                    break;
                }
                ret.resize(res);
            }
            clearError(PTL_ERROR_REF(err));
            break;
        }
        return ret;
    }

    #if PTL_HAVE_SETGROUPS

    void setGroups(std::span<const gid_t> groups, PTL_ERROR_REF_ARG(err)) 
    requires(PTL_ERROR_REQ(err)) {
        if (auto size = groups.size(); size > size_t(std::numeric_limits<int>::max())) 
            throwErrorCode(EINVAL, "number of groups: {} exceeds int", size);

        if (::setgroups(int(groups.size()), groups.data()) != 0) 
            handleError(PTL_ERROR_REF(err), errno, "setgroups() failed");
        else
            clearError(PTL_ERROR_REF(err));
    }

    void setGroups(std::initializer_list<const gid_t> groups, PTL_ERROR_REF_ARG(err)) 
    requires(PTL_ERROR_REQ(err)) {
        setGroups(std::span<const gid_t>(groups), PTL_ERROR_REF(err));
    }

    #endif

}

#endif

#endif 
