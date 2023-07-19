// Copyright (c) 2023, Eugene Gershnik
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PTL_HEADER_SOCKET_H_INCLUDED
#define PTL_HEADER_SOCKET_H_INCLUDED

#include <ptl/core.h>
#include <ptl/file.h>
#include <ptl/util.h>


#if __has_include(<sys/socket.h>)
    #include <sys/socket.h>
#endif
#if __has_include(<netinet/in.h>)
    #include <netinet/in.h>
#endif

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

namespace ptl::inline v0 {

    #ifndef _WIN32

        template<class T>
        concept SocketLike = FileDescriptorLike<T>;

        template<SocketLike T>
        [[gnu::always_inline]] inline int c_socket(T && obj)
            { return c_fd(std::forward<T>(obj)); }

        using Socket = FileDescriptor;

        using socklen_t = ::socklen_t;

        namespace impl {
            using SocketOptionValueType = void;
        }

    #else

        template<class T> struct SocketTraits;

        template<class T>
        concept SocketLike = requires(T && obj) {
            { SocketTraits<std::remove_cvref_t<T>>::c_socket(std::forward<T>(obj)) } -> SameAs<SOCKET>;
        };

        template<SocketLike T>
        [[gnu::always_inline]] inline SOCKET c_socket(T && obj)
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
            Socket(Socket && src) noexcept : m_socket(src.m_socket) {
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
            
            auto get() const noexcept -> SOCKET {
                return m_socket;
            }
            
            auto detach() noexcept -> SOCKET {
                SOCKET ret = m_socket;
                m_socket = INVALID_SOCKET;
                return ret;
            }
            
            explicit operator bool() const noexcept {
                return m_socket != INVALID_SOCKET;
            }
        private:
            SOCKET m_socket = INVALID_SOCKET;
        };

        template<> struct SocketTraits<int> {
            [[gnu::always_inline]] static SOCKET c_socket(SOCKET sock) noexcept
                { return sock;}
        };
        template<> struct SocketTraits<Socket> {
            [[gnu::always_inline]] static SOCKET c_socket(const Socket & sock) noexcept
                { return sock.get();}
        };

        using socklen_t = int;

        namespace impl {
            using SocketOptionValueType = char;
        }

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

    inline void setSocketOption(SocketLike auto && socket, 
                                int level, int option_name, const void * option_value, socklen_t option_len,
                                PTL_ERROR_REF_ARG(err)) 
    requires(PTL_ERROR_REQ(err)) {
        auto fd = c_socket(std::forward<decltype(socket)>(socket));
        int res = ::setsockopt(fd, level, option_name, static_cast<const impl::SocketOptionValueType *>(option_value), option_len);
        if (res != 0)
            handleError(PTL_ERROR_REF(err), impl::getSocketError(), "setsockopt({}, {}, {}) failed", fd, level, option_name);
        else
            clearError(PTL_ERROR_REF(err));
    }

    inline void getSocketOption(SocketLike auto && socket, 
                                int level, int option_name, void * option_value, socklen_t * option_len,
                                PTL_ERROR_REF_ARG(err)) 
    requires(PTL_ERROR_REQ(err)) {
        auto fd = c_socket(std::forward<decltype(socket)>(socket));
        int res = ::getsockopt(fd, level, option_name, static_cast<impl::SocketOptionValueType *>(option_value), option_len);
        if (res != 0)
            handleError(PTL_ERROR_REF(err), impl::getSocketError(), "setsockopt({}, {}, {}) failed", fd, level, option_name);
        else
            clearError(PTL_ERROR_REF(err));
    }

    template<class T>
    inline void setSocketOption(SocketLike auto && socket, 
                                int level, int option_name, const T & value,
                                PTL_ERROR_REF_ARG(err)) 
    requires(PTL_ERROR_REQ(err)) {
        if constexpr (std::is_same_v<T, bool>) {
            const int val = value;
            setSocketOption(std::forward<decltype(socket)>(socket), level, option_name, &val, socklen_t(sizeof(val)), PTL_ERROR_REF(err));
        } else {
            setSocketOption(std::forward<decltype(socket)>(socket), level, option_name, &value, socklen_t(sizeof(value)), PTL_ERROR_REF(err));
        }
    }

    template<class T>
    inline void getSocketOption(SocketLike auto && socket, 
                                int level, int option_name, T & value,
                                PTL_ERROR_REF_ARG(err)) 
    requires(PTL_ERROR_REQ(err)) {
        socklen_t len;
        if constexpr (std::is_same_v<T, bool>) {
            int val;
            len = socklen_t(sizeof(val));
            getSocketOption(std::forward<decltype(socket)>(socket), level, option_name, &val, &len, PTL_ERROR_REF(err));
            if (len == socklen_t(sizeof(val))) {
                value = val;
                return;
            }
            #ifdef _WIN32
                if (len == 1) { //Windows is moronic this way 
                    value = *((const unsigned char *)&val) != 0;
                    return;
                }
            #endif
        } else {
            len = socklen_t(sizeof(value));
            getSocketOption(std::forward<decltype(socket)>(socket), level, option_name, &value, &len, PTL_ERROR_REF(err));
            if (len == socklen_t(sizeof(value))) {
                return;
            }
        }
        throwErrorCode(EINVAL, "return length from getsockopt({}, {}): {} doesn't match passed argument", level, option_name, len);
    }

    template<class T>
    inline auto getSocketOption(SocketLike auto && socket, 
                                int level, int option_name,
                                PTL_ERROR_REF_ARG(err)) -> T
    requires(PTL_ERROR_REQ(err)) {
        T ret;
        getSocketOption(std::forward<decltype(socket)>(socket), level, option_name, ret, PTL_ERROR_REF(err));
        return ret;
    }

    template<class Allowed1, class... AllowedRest>
    struct SockOptDesc {
        const int level;
        const int name;  
    };

    template<class T, class Allowed1, class... AllowedRest>
    inline void setSocketOption(SocketLike auto && socket, SockOptDesc<Allowed1, AllowedRest...> desc, const T & option,
                                PTL_ERROR_REF_ARG(err)) 
    requires(PTL_ERROR_REQ(err) && TypeInPack<T, Allowed1, AllowedRest...>) {
        setSocketOption(std::forward<decltype(socket)>(socket), desc.level, desc.name, option,
                        PTL_ERROR_REF(err));
    }

    template<class T, class Allowed1, class... AllowedRest>
    inline void getSocketOption(SocketLike auto && socket, SockOptDesc<Allowed1, AllowedRest...> desc, T & option,
                                PTL_ERROR_REF_ARG(err))
    requires(PTL_ERROR_REQ(err) && TypeInPack<T, Allowed1, AllowedRest...>) {
        getSocketOption(std::forward<decltype(socket)>(socket), desc.level, desc.name, option,
                        PTL_ERROR_REF(err));
    }

    template<class Allowed1, class T = Allowed1, class... AllowedRest>
    inline auto getSocketOption(SocketLike auto && socket, SockOptDesc<Allowed1, AllowedRest...> desc,
                                PTL_ERROR_REF_ARG(err)) -> T
    requires(PTL_ERROR_REQ(err) && TypeInPack<T, Allowed1, AllowedRest...>) {
        T ret;
        getSocketOption(std::forward<decltype(socket)>(socket), desc, ret, PTL_ERROR_REF(err));
        return ret;
    }

    //Posix options

    constexpr auto SockOptDebug              = SockOptDesc<bool>            {SOL_SOCKET, SO_DEBUG};
    constexpr auto SockOptBroadcast          = SockOptDesc<bool>            {SOL_SOCKET, SO_BROADCAST};
    constexpr auto SockOptReuseAddr          = SockOptDesc<bool>            {SOL_SOCKET, SO_REUSEADDR};
    constexpr auto SockOptKeepAlive          = SockOptDesc<bool>            {SOL_SOCKET, SO_KEEPALIVE};
    constexpr auto SockOptLinger             = SockOptDesc<::linger>        {SOL_SOCKET, SO_LINGER};
    constexpr auto SockOptOOBInline          = SockOptDesc<bool>            {SOL_SOCKET, SO_OOBINLINE};
    constexpr auto SockOptSndBuf             = SockOptDesc<unsigned, int>   {SOL_SOCKET, SO_SNDBUF};
    constexpr auto SockOptRcvBuf             = SockOptDesc<unsigned, int>   {SOL_SOCKET, SO_RCVBUF};
    constexpr auto SockOptDontRoute          = SockOptDesc<bool>            {SOL_SOCKET, SO_DONTROUTE};
    constexpr auto SockOptRcvLowWatermark    = SockOptDesc<unsigned, int>   {SOL_SOCKET, SO_RCVLOWAT};
    constexpr auto SockOptRcvTimeout         = SockOptDesc<::timeval>       {SOL_SOCKET, SO_RCVLOWAT};
    constexpr auto SockOptSndLowWatermark    = SockOptDesc<unsigned, int>   {SOL_SOCKET, SO_SNDLOWAT};
    constexpr auto SockOptSndTimeout         = SockOptDesc<::timeval>       {SOL_SOCKET, SO_SNDTIMEO};

    //Non-standard options

    #ifdef SO_REUSEPORT
        constexpr auto SockOptReusePort          = SockOptDesc<bool>        {SOL_SOCKET, SO_REUSEPORT};
    #endif
    #ifdef SO_TYPE
        constexpr auto SockOptType               = SockOptDesc<int>         {SOL_SOCKET, SO_TYPE};
    #endif
    #ifdef SO_ERROR
        constexpr auto SockOptError              = SockOptDesc<int>         {SOL_SOCKET, SO_ERROR};
    #endif
    #ifdef SO_NOSIGPIPE
        constexpr auto SockOptNoSIGPIPE          = SockOptDesc<bool>        {SOL_SOCKET, SO_NOSIGPIPE};
    #endif
    #ifdef SO_NREAD
        constexpr auto SockOptNRead              = SockOptDesc<socklen_t>   {SOL_SOCKET, SO_NREAD};
    #endif
    #ifdef SO_NWRITE
        constexpr auto SockOptNWrite             = SockOptDesc<socklen_t>   {SOL_SOCKET, SO_NWRITE};
    #endif
    #ifdef SO_LINGER_SEC
        constexpr auto SockOptLingerSec          = SockOptDesc<unsigned, 
                                                               int>         {SOL_SOCKET, SO_LINGER_SEC};
    #endif
    #ifdef SO_ACCEPTCONN
        constexpr auto SockOptAcceptsConn        = SockOptDesc<bool>        {SOL_SOCKET, SO_ACCEPTCONN};
    #endif
    #ifdef SO_TIMESTAMP
        constexpr auto SockOptTimestamp          = SockOptDesc<bool>        {SOL_SOCKET, SO_TIMESTAMP};
    #endif
    #ifdef SO_TIMESTAMPNS
        constexpr auto SockOptTimestampNs        = SockOptDesc<bool>        {SOL_SOCKET, SO_TIMESTAMPNS};
    #endif
    #ifdef SO_TIMESTAMP_MONOTONIC
        constexpr auto SockOptTimestampMonotonic = SockOptDesc<bool>        {SOL_SOCKET, SO_TIMESTAMP_MONOTONIC};
    #endif
    #ifdef SO_DOMAIN
        constexpr auto SockOptDomain             = SockOptDesc<int>         {SOL_SOCKET, SO_DOMAIN};
    #endif
    #ifdef SO_PROTOCOL
        constexpr auto SockOptProtocols          = SockOptDesc<int>         {SOL_SOCKET, SO_PROTOCOL};
    #endif
    #ifdef SO_BSP_STATE
        constexpr auto SockOptBspState           = SockOptDesc<CSADDR_INFO> {SOL_SOCKET, SO_BSP_STATE};
    #endif
    #ifdef SO_EXCLUSIVEADDRUSE
        constexpr auto SockOptExclusiveAddrUse   = SockOptDesc<bool>        {SOL_SOCKET, SO_EXCLUSIVEADDRUSE};
    #endif

    //IPPROTO_IP options
    #ifdef IP_MULTICAST_LOOP
        constexpr auto SockOptIPv4MulticastLoop     = SockOptDesc<bool>     {IPPROTO_IP, IP_MULTICAST_LOOP};
    #endif
    #ifdef IP_MULTICAST_TTL
        constexpr auto SockOptIPv4MulticastTtl      = SockOptDesc<uint8_t,
                                                                  int8_t>   {IPPROTO_IP, IP_MULTICAST_TTL};
    #endif
    #ifdef IP_MULTICAST_IF
        constexpr auto SockOptIPv4MulticastIface    = SockOptDesc<
                                                        #if PTL_HAVE_IP_MREQN
                                                            ::ip_mreqn,
                                                        #endif
                                                        #if PTL_HAVE_IP_MREQ
                                                            ::ip_mreq,
                                                        #endif
                                                        ::in_addr>          {IPPROTO_IP, IP_MULTICAST_IF};
    #endif
    #ifdef IP_ADD_MEMBERSHIP
        constexpr auto SockOptIPv4AddMembership     = SockOptDesc<
                                                        #if PTL_HAVE_IP_MREQN
                                                            ::ip_mreqn,
                                                        #endif
                                                        ::ip_mreq>          {IPPROTO_IP, IP_ADD_MEMBERSHIP};
    #endif
    #ifdef IP_DROP_MEMBERSHIP
        constexpr auto SockOptIPv4DropMembership    = SockOptDesc<
                                                        #if PTL_HAVE_IP_MREQN
                                                            ::ip_mreqn,
                                                        #endif
                                                        ::ip_mreq>          {IPPROTO_IP, IP_DROP_MEMBERSHIP};
    #endif
    #ifdef IP_MULTICAST_ALL
        constexpr auto SockOptIPv4MulticastAll      = SockOptDesc<bool>     {IPPROTO_IP, IP_MULTICAST_ALL};
    #endif

    //IPPROTO_IPV6 options
    #ifdef IPV6_MULTICAST_LOOP
        constexpr auto SockOptIPv6MulticastLoop     = SockOptDesc<bool>     {IPPROTO_IPV6, IPV6_MULTICAST_LOOP};
    #endif
    #ifdef IPV6_MULTICAST_HOPS
        constexpr auto SockOptIPv6MulticastHops     = SockOptDesc<int>      {IPPROTO_IPV6, IPV6_MULTICAST_HOPS};
    #endif
    #ifdef IPV6_MULTICAST_IF
        constexpr auto SockOptIPv6MulticastIface    = SockOptDesc<unsigned, 
                                                                  int>      {IPPROTO_IPV6, IPV6_MULTICAST_IF};
    #endif
    #ifdef IPV6_MULTICAST_ALL
        constexpr auto SockOptIPv6MulticastAll      = SockOptDesc<bool>     {IPPROTO_IPV6, IPV6_MULTICAST_ALL};
    #endif

}

#endif
