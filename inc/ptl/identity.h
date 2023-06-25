#ifndef PTL_HEADER_IDENTITY_H_INCLUDED
#define PTL_HEADER_IDENTITY_H_INCLUDED

#include <ptl/core.h>

#include <utility>

#ifndef _WIN32

namespace ptl::inline v0 {

    class Identity {
    public:
        uid_t uid;
        gid_t gid;

        Identity(uid_t uid_, gid_t gid_) : uid(uid_), gid(gid_) 
        {}
        
        static auto real() -> Identity {
            return Identity(getuid(), getgid());
        }
        static auto effective() -> Identity {
            return Identity(geteuid(), getegid());
        }

        void setReal(PTL_ERROR_REF_ARG(err)) const noexcept(PTL_ERROR_NOEXCEPT(err))
        requires(PTL_ERROR_REQ(err)) {
            clearError(PTL_ERROR_REF(err));
            if (setgid(this->gid) != 0) {
                handleError(PTL_ERROR_REF(err), errno, "setgid({}) failed", this->gid);
            } else if (setuid(this->uid) != 0) {
                handleError(PTL_ERROR_REF(err), errno, "setuid({}) failed", this->uid);
            }
        }

        void setEffective(PTL_ERROR_REF_ARG(err)) const noexcept(PTL_ERROR_NOEXCEPT(err)) 
        requires(PTL_ERROR_REQ(err)) {
            clearError(PTL_ERROR_REF(err));
            if (setegid(this->gid) != 0) {
                handleError(PTL_ERROR_REF(err), errno, "setegid({}) failed", this->gid);
            } else if (seteuid(this->uid) != 0) {
                handleError(PTL_ERROR_REF(err), errno, "seteuid({}) failed", this->uid);
            }
        }
    };

}

#endif

#endif 
