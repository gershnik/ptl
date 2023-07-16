// Copyright (c) 2023, Eugene Gershnik
// SPDX-License-Identifier: BSD-3-Clause

#include "common.h"

#include <ptl/spawn.h>

#ifdef __MINGW32__

#include <winerror.h>

#endif

using namespace ptl;

#ifdef __MINGW32__

constexpr struct { 
    int win32;
    std::errc posix;
} errorsMap[] = {
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


bool SystemErrorMatcher::match(const std::system_error & ex) const {
#ifdef __MINGW32__
    if (ex.code().category() == std::system_category()) {
        for (const auto & entry : errorsMap) {
            if (entry.win32 == ex.code().value()) {
                return entry.posix == m_code;
            }
        }
        return false;
    }
#endif
    return ex.code() == m_code;
}



#ifndef _WIN32

std::string shell(const StringRefArray & args) {
    auto [readPipe, writePipe] = Pipe::create();

    #if !defined(__ANDROID__) || (defined(__ANDROID__) && __ANDROID_API__ >= 28)
        SpawnFileActions fa;
        fa.addClose(readPipe);
        fa.addDuplicateTo(writePipe, stdout);
        auto child = spawn(args, SpawnSettings().fileActions(fa).usePath());
    #else 
        auto child = forkProcess();
        if (!child) {
            readPipe.close();
            duplicateTo(writePipe, stdout);
            execp(args);
        }
    #endif
    writePipe.close();
    auto ret = rtrim(readAll(readPipe));
    child.wait();
    return ret;
}

#endif
