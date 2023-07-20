// Copyright (c) 2023, Eugene Gershnik
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PTL_HEADER_ERRORS_H_INCLUDED
#define PTL_HEADER_ERRORS_H_INCLUDED

#include <ptl/core.h>

namespace ptl::inline v0 {

    template<Error First, Error... Rest>
    class AllowedErrors {
    public:
        auto code() const noexcept -> Error 
            { return m_code; }

        auto assign(Error err) noexcept -> bool {
            for(auto allowed: {First, Rest...}) {
                if (err == allowed) {
                    m_code = err;
                    return true;
                }
            }
            return false;
        }

        void clear() noexcept 
            { m_code = 0; }

        explicit operator bool() const noexcept 
            { return bool(m_code); }
    private:
        Error m_code;
    };

    template<Error First, Error... Rest> 
    struct ErrorTraits<AllowedErrors<First, Rest...>> {
        template<class... T>
        [[gnu::always_inline]] static inline void assignError(AllowedErrors<First, Rest...> & dest, Error err, const char * format, T && ...args) {
            if (!dest.assign(err))
                ptl::throwErrorCode(err, format, std::forward<T>(args)...);
        }
        
        [[gnu::always_inline]] static inline void clearError(AllowedErrors<First, Rest...> & err) noexcept {
            err.clear();
        }

        [[gnu::always_inline]] static inline auto failed(const AllowedErrors<First, Rest...> & err) noexcept -> bool {
            return bool(err);
        }
    };


    #ifdef _WIN32

    constexpr struct { 
        int win32;
        std::errc posix;
    } systemErrorsMap[] = {
        {ERROR_INVALID_FUNCTION, std::errc::function_not_supported},
        {ERROR_FILE_NOT_FOUND, std::errc::no_such_file_or_directory},
        {ERROR_PATH_NOT_FOUND, std::errc::no_such_file_or_directory},
        {ERROR_TOO_MANY_OPEN_FILES, std::errc::too_many_files_open},
        {ERROR_ACCESS_DENIED, std::errc::permission_denied},
        {ERROR_INVALID_HANDLE, std::errc::invalid_argument},
        {ERROR_NOT_ENOUGH_MEMORY, std::errc::not_enough_memory},
        {ERROR_INVALID_ACCESS, std::errc::permission_denied},
        {ERROR_OUTOFMEMORY, std::errc::not_enough_memory},
        {ERROR_INVALID_DRIVE, std::errc::no_such_device},
        {ERROR_CURRENT_DIRECTORY, std::errc::permission_denied},
        {ERROR_NOT_SAME_DEVICE, std::errc::cross_device_link},
        {ERROR_WRITE_PROTECT, std::errc::permission_denied},
        {ERROR_BAD_UNIT, std::errc::no_such_device},
        {ERROR_NOT_READY, std::errc::resource_unavailable_try_again},
        {ERROR_SEEK, std::errc::io_error},
        {ERROR_WRITE_FAULT, std::errc::io_error},
        {ERROR_READ_FAULT, std::errc::io_error},
        {ERROR_SHARING_VIOLATION, std::errc::permission_denied},
        {ERROR_LOCK_VIOLATION, std::errc::no_lock_available},
        {ERROR_HANDLE_DISK_FULL, std::errc::no_space_on_device},
        {ERROR_NOT_SUPPORTED, std::errc::not_supported},
        {ERROR_BAD_NETPATH, std::errc::no_such_file_or_directory},
        {ERROR_DEV_NOT_EXIST, std::errc::no_such_device},
        {ERROR_BAD_NET_NAME, std::errc::no_such_file_or_directory},
        {ERROR_FILE_EXISTS, std::errc::file_exists},
        {ERROR_CANNOT_MAKE, std::errc::permission_denied},
        {ERROR_INVALID_PARAMETER, std::errc::invalid_argument},
        {ERROR_BROKEN_PIPE, std::errc::broken_pipe},
        {ERROR_OPEN_FAILED, std::errc::io_error},
        {ERROR_BUFFER_OVERFLOW, std::errc::filename_too_long},
        {ERROR_DISK_FULL, std::errc::no_space_on_device},
        {ERROR_SEM_TIMEOUT, std::errc::timed_out},
        {ERROR_INVALID_NAME, std::errc::no_such_file_or_directory},
        {ERROR_NEGATIVE_SEEK, std::errc::invalid_argument},
        {ERROR_BUSY_DRIVE, std::errc::device_or_resource_busy},
        {ERROR_DIR_NOT_EMPTY, std::errc::directory_not_empty},
        {ERROR_BUSY, std::errc::device_or_resource_busy},
        {ERROR_ALREADY_EXISTS, std::errc::file_exists},
        {ERROR_FILENAME_EXCED_RANGE, std::errc::filename_too_long},
        {ERROR_LOCKED, std::errc::no_lock_available},
        {WAIT_TIMEOUT, std::errc::timed_out},
        {ERROR_DIRECTORY, std::errc::invalid_argument},
        {ERROR_OPERATION_ABORTED, std::errc::operation_canceled},
        {ERROR_NOACCESS, std::errc::permission_denied},
        {ERROR_CANTOPEN, std::errc::io_error},
        {ERROR_CANTREAD, std::errc::io_error},
        {ERROR_CANTWRITE, std::errc::io_error},
        {ERROR_RETRY, std::errc::resource_unavailable_try_again},
        {ERROR_TIMEOUT, std::errc::timed_out},
        {ERROR_OPEN_FILES, std::errc::device_or_resource_busy},
        {ERROR_DEVICE_IN_USE, std::errc::device_or_resource_busy},
        {ERROR_REPARSE_TAG_INVALID, std::errc::invalid_argument},
        {WSAEINTR, std::errc::interrupted},
        {WSAEBADF, std::errc::bad_file_descriptor},
        {WSAEACCES, std::errc::permission_denied},
        {WSAEFAULT, std::errc::bad_address},
        {WSAEINVAL, std::errc::invalid_argument},
        {WSAEMFILE, std::errc::too_many_files_open},
        {WSAEWOULDBLOCK, std::errc::operation_would_block},
        {WSAEINPROGRESS, std::errc::operation_in_progress},
        {WSAEALREADY, std::errc::connection_already_in_progress},
        {WSAENOTSOCK, std::errc::not_a_socket},
        {WSAEDESTADDRREQ, std::errc::destination_address_required},
        {WSAEMSGSIZE, std::errc::message_size},
        {WSAEPROTOTYPE, std::errc::wrong_protocol_type},
        {WSAENOPROTOOPT, std::errc::no_protocol_option},
        {WSAEPROTONOSUPPORT, std::errc::protocol_not_supported},
        {WSAEOPNOTSUPP, std::errc::operation_not_supported},
        {WSAEAFNOSUPPORT, std::errc::address_family_not_supported},
        {WSAEADDRINUSE, std::errc::address_in_use},
        {WSAEADDRNOTAVAIL, std::errc::address_not_available},
        {WSAENETDOWN, std::errc::network_down},
        {WSAENETUNREACH, std::errc::network_unreachable},
        {WSAENETRESET, std::errc::network_reset},
        {WSAECONNABORTED, std::errc::connection_aborted},
        {WSAECONNRESET, std::errc::connection_reset},
        {WSAENOBUFS, std::errc::no_buffer_space},
        {WSAEISCONN, std::errc::already_connected},
        {WSAENOTCONN, std::errc::not_connected},
        {WSAETIMEDOUT, std::errc::timed_out},
        {WSAECONNREFUSED, std::errc::connection_refused},
        {WSAENAMETOOLONG, std::errc::filename_too_long},
        {WSAEHOSTUNREACH, std::errc::host_unreachable},
    };

    #endif


    inline auto errorEquals(const std::error_code & ec, std::errc code) -> bool {
        //Avoid virtual function calls and other nonsense for generic and system categories
        if (ec.category() == std::generic_category())
            return ec.value() == static_cast<int>(code);
        if (ec.category() == std::system_category()) {
            #ifndef _WIN32
                return ec.value() == static_cast<int>(code);
            #else
                for (const auto & entry : systemErrorsMap) {
                    if (entry.win32 == ec.value()) {
                        return entry.posix == code;
                    }
                }
                return false;
            #endif
        }
        return ec == code;
    }
}


#endif