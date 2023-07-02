#ifndef PTL_HEADER_USERS_H_INCLUDED
#define PTL_HEADER_USERS_H_INCLUDED

#include <ptl/core.h>
#include <ptl/system.h>

#include <optional>
#include <vector>
#include <utility>

#ifndef _WIN32

#include <pwd.h>
#include <grp.h>

namespace ptl::inline v0 {

    template<class Traits>
    class UserInfoImpl : public Traits::CStruct {
    public:
        UserInfoImpl() noexcept : Traits::CStruct{} 
        {}
        UserInfoImpl(const UserInfoImpl &) = delete;
        UserInfoImpl & operator=(const UserInfoImpl &) = delete;

        UserInfoImpl(UserInfoImpl &&) noexcept = default;
        UserInfoImpl & operator=(UserInfoImpl &&) noexcept = default;

        static auto getByName(StringLike auto && name, 
                              PTL_ERROR_REF_ARG(err)) -> std::optional<UserInfoImpl> 
        requires(PTL_ERROR_REQ(err)) {
            auto nameStr = c_str(std::forward<decltype(name)>(name));
            return getByFunc(nameStr, PTL_ERROR_REF(err));
        }

        static auto getById(typename Traits::IDType id, 
                            PTL_ERROR_REF_ARG(err)) -> std::optional<UserInfoImpl> 
        requires(PTL_ERROR_REQ(err)) {
            return getByFunc(id, PTL_ERROR_REF(err));
        }

    private:
        UserInfoImpl(size_t buflen): m_buf(buflen) {}

        static auto startBufSize() -> size_t {
            static size_t theSize = systemConfig(Traits::MaxSizeConf).value_or(4096);
            return theSize;
        }

        static auto getByFunc(auto byArg, PTL_ERROR_REF_ARG(err)) -> std::optional<UserInfoImpl> 
        requires(PTL_ERROR_REQ(err)) {

            auto [getFunc, getFuncName] = Traits::getByInfo(byArg);
            clearError(PTL_ERROR_REF(err));
            UserInfoImpl ret(startBufSize());
            for ( ; ; ) {
                typename Traits::CStruct * result = nullptr;
                int res = getFunc(byArg, &ret, ret.m_buf.data(), ret.m_buf.size(), &result);
                if (res == 0) {
                    if (!result)
                        break;
                    return ret;
                }
                
                if (res != ERANGE) {
                    handleError(PTL_ERROR_REF(err), res, "{}({}) failed", getFuncName, byArg);
                    break;
                }
                ret.m_buf.resize(ret.m_buf.size() + 4096);
            }
            return std::nullopt;
        }
    private:
        std::vector<char> m_buf;
    };

    struct PasswdTraits {
        using CStruct = ::passwd;
        using IDType = ::uid_t;
        static constexpr int  MaxSizeConf = _SC_GETPW_R_SIZE_MAX;
        
        static auto getByInfo(const char *) {
            return std::make_pair(getpwnam_r, "getpwnam_r");
        }
        static auto getByInfo(::uid_t) {
            return std::make_pair(getpwuid_r, "getpwuid_r");
        }
    };

    struct GroupTraits {
        using CStruct = ::group;
        using IDType = ::gid_t;
        static constexpr int  MaxSizeConf = _SC_GETGR_R_SIZE_MAX;
        
        static auto getByInfo(const char *) {
            return std::make_pair(getgrnam_r, "getgrnam_r");
        }
        static auto getByInfo(::uid_t) {
            return std::make_pair(getgrgid_r, "getgrgid_r");
        }
    };


    using Passwd = UserInfoImpl<PasswdTraits>;
    using Group = UserInfoImpl<GroupTraits>;

}

#endif

#endif
