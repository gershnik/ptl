// Copyright (c) 2023, Eugene Gershnik
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PTL_HEADER_SIGNAL_H_INCLUDED
#define PTL_HEADER_SIGNAL_H_INCLUDED

#include <ptl/core.h>
#include <ptl/file.h>

#if __has_include(<sys/socket.h>)
    #include <sys/socket.h>
#endif

#ifdef _WIN32
#include <winsock.h>
#endif

namespace ptl::inline v0 {

    #ifndef _WIN32

        template<class T>
        concept SocketLike = FileDescriptorLike<T>;

        template<SocketLike T>
        [[gnu::always_inline]] inline int c_socket(T && obj)
            { return c_fd(std::forward<T>(obj)); }

        using Socket = FileDescriptor;

    #else

        template<class T> struct SocketTraits;

        template<class T>
        concept SocketLike = requires(T && obj) {
            { SocketTraits<std::remove_cvref_t<T>>::c_socket(std::forward<T>(obj)) } -> SameAs<SOCKET>;
        };

        template<FileDescriptorLike T>
        [[gnu::always_inline]] inline int c_socket(T && obj)
            { return SocketTraits<std::remove_cvref_t<T>>::c_socket(std::forward<T>(obj)); }


        class Socket {
        public:
            Socket() noexcept = default;
            explicit Socket(SOCKET sock) noexcept : m_socket(sock) {
            }
            ~Socket() noexcept {
                if (m_socket != INVALID_SOCKET)
                    closesocket(m_socket);
            }
            Socket(Socket && src) noexcept : m_fd(src.m_fd) {
                src.m_socket = INVALID_SOCKET;
            }
            Socket(const Socket &) = delete;
            Socket & operator=(Socket src) noexcept {
                swap(src, *this);
                return *this;
            }
            
            void close() noexcept {
                *this = Socket();
            }

            friend void swap(Socket & lhs, Socket & rhs) noexcept {
                std::swap(lhs.m_socket, rhs.m_socket);
            }
            
            auto get() const noexcept -> int {
                return m_socket;
            }
            
            auto detach() noexcept -> int {
                int ret = m_socket;
                m_socket = INVALID_SOCKET;
                return ret;
            }
            
            explicit operator bool() const noexcept {
                return m_socket != INVALID_SOCKET;
            }
        private:
            SOCKET m_socket = INVALID_SOCKET;
        };

    #endif

    namespace impl {
        inline auto getSocketError() -> SystemError {
            #ifndef _WIN32
                return errno;
            #else
                return {WSAGetLastError()};
            #endif
        }
    }

    inline auto createSocket(int domain, int type, int protocol,
                             PTL_ERROR_REF_ARG(err)) -> Socket 
    requires(PTL_ERROR_REQ(err)) {
        Socket ret(::socket(domain, type, protocol));
        if (!ret) 
            handleError(PTL_ERROR_REF(err), impl::getSocketError(), "socket({}, {}, {}) failed", domain, type, protocol);
        else
            clearError(PTL_ERROR_REF(err));
        return ret;
    }

    inline void setSocketOption(FileDescriptorLike auto && socket, 
                                int level, int option_name, const void * option_value, socklen_t option_len,
                                PTL_ERROR_REF_ARG(err)) 
    requires(PTL_ERROR_REQ(err)) {
        auto fd = c_socket(std::forward<decltype(socket)>(socket));
        int res = ::setsockopt(fd, level, option_name, option_value, option_len);
        if (res != 0)
            handleError(PTL_ERROR_REF(err), impl::getSocketError(), "setsockopt({}, {}, {}) failed", fd, level, option_name);
        else
            clearError(PTL_ERROR_REF(err));
    }

    inline void getSocketOption(FileDescriptorLike auto && socket, 
                                int level, int option_name, void * option_value, socklen_t option_len,
                                PTL_ERROR_REF_ARG(err)) 
    requires(PTL_ERROR_REQ(err)) {
        auto fd = c_socket(std::forward<decltype(socket)>(socket));
        int res = ::getsockopt(fd, level, option_name, option_value, option_len);
        if (res != 0)
            handleError(PTL_ERROR_REF(err), impl::getSocketError(), "setsockopt({}, {}, {}) failed", fd, level, option_name);
        else
            clearError(PTL_ERROR_REF(err));
    }

    template<class T>
    concept SocketOption = requires(const T & cobj, T & mobj) {
    
        { T::level } -> SameAs<int>;
        { T::name } -> SameAs<int>;
        { cobj.addr() } -> ConvertibleTo<const void *>;
        { mobj.addr() } -> ConvertibleTo<void *>;
        { cobj.size() } -> SameAs<socklen_t>;
        { mobj.size() } -> SameAs<socklen_t>;
    };

    template<SocketOption Option>
    inline void setSocketOption(FileDescriptorLike auto && socket, const Option & option,
                                PTL_ERROR_REF_ARG(err)) 
    requires(PTL_ERROR_REQ(err)) {
        setSocketOption(std::forward<decltype(socket)>(socket), Option::level, Option::name, option.addr(), option.size(),
                        PTL_ERROR_REF(err));
    }

    template<SocketOption Option>
    inline void getSocketOption(FileDescriptorLike auto && socket, Option & option,
                                PTL_ERROR_REF_ARG(err)) 
    requires(PTL_ERROR_REQ(err)) {
        getSocketOption(std::forward<decltype(socket)>(socket), Option::level, Option::name, option.addr(), option.size(),
                        PTL_ERROR_REF(err));
    }

    template<SocketOption Option>
    inline auto getSocketOption(FileDescriptorLike auto && socket,
                                PTL_ERROR_REF_ARG(err)) -> Option
    requires(PTL_ERROR_REQ(err)) {
        Option option;
        getSocketOption(std::forward<decltype(socket)>(socket), Option::level, Option::name, option.addr(), option.size(),
                        PTL_ERROR_REF(err));
        return option;
    }

    template<int Level, int Name, class T, bool IsPrimitive = !std::is_class_v<T> && !std::is_union_v<T>>
    class SockOpt;

    template<int Level, int Name, class T>
    class SockOpt<Level, Name, T, /*IsPrimitive*/true> {
    public:
        static constexpr int level = Level;
        static constexpr int name  = Name;

        constexpr auto value() const noexcept -> T
            { return m_value; }

        auto addr() const noexcept -> const void *
            { return &this->m_value; }
        auto addr() noexcept -> void *
            { return &this->m_value; }
        static auto size() noexcept -> socklen_t
            { return socklen_t(sizeof(T)); } 
    private:
        T m_value;
    };

    template<int Level, int Name>
    class SockOpt<Level, Name, bool, /*IsPrimitive*/true> {
    public:
        static constexpr int level = Level;
        static constexpr int name  = Name;

        constexpr SockOpt(bool val) noexcept: m_value(val)
            {}

        constexpr auto value() const noexcept -> bool
            { return m_value != 0; }

        auto addr() const noexcept -> const void *
            { return &this->m_value; }
        auto addr() noexcept -> void *
            { return &this->m_value; }
        static auto size() noexcept -> socklen_t
            { return socklen_t(sizeof(int)); } 
    private:
        int m_value;
    };

    template<int Level, int Name, class T>
    class SockOpt<Level, Name, T, /*IsPrimitive*/false> : T {
    public:
        static constexpr int level = Level;
        static constexpr int name  = Name;

        SockOpt() noexcept : T{}
        {}

        auto addr() const noexcept -> const void * 
            { return static_cast<const T *>(this); }
        auto addr() noexcept -> void * 
            { return static_cast<const T *>(this); }
        static auto size() noexcept -> socklen_t 
            { return socklen_t(sizeof(T)); }
    };

    using SockOptDebug              = SockOpt<SOL_SOCKET, SO_DEBUG,         bool>;
    using SockOptBroadcast          = SockOpt<SOL_SOCKET, SO_BROADCAST,     bool>;
    using SockOptReuseAddr          = SockOpt<SOL_SOCKET, SO_REUSEADDR,     bool>;
    using SockOptKeepAlive          = SockOpt<SOL_SOCKET, SO_KEEPALIVE,     bool>;
    using SockOptLinger             = SockOpt<SOL_SOCKET, SO_LINGER,        ::linger>;
    using SockOptOOBInline          = SockOpt<SOL_SOCKET, SO_OOBINLINE,     bool>;
    using SockOptSndBuf             = SockOpt<SOL_SOCKET, SO_SNDBUF,        int>;
    using SockOptRcvBuf             = SockOpt<SOL_SOCKET, SO_RCVBUF,        int>;
    using SockOptDontRoute          = SockOpt<SOL_SOCKET, SO_DONTROUTE,     bool>;
    using SockOptRcvLowWatermark    = SockOpt<SOL_SOCKET, SO_RCVLOWAT,      int>;
    using SockOptRcvTimeout         = SockOpt<SOL_SOCKET, SO_RCVLOWAT,      ::timeval>;
    using SockOptSndLowWatermark    = SockOpt<SOL_SOCKET, SO_SNDLOWAT,      int>;
    using SockOptSndTimeout         = SockOpt<SOL_SOCKET, SO_SNDTIMEO,      ::timeval>;
}

#endif
