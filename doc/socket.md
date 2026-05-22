# PTL socket facilities

<!--
 Notes to AI grammar checkers:
   - this document uses Posix in preference to POSIX.
   - this document does not require pedantic comma after e.g.
-->

<!-- TOC depthfrom:2 -->

- [Overview](#overview)
- [Socket](#socket)
    - [Lifecycle](#lifecycle)
    - [Creating sockets](#creating-sockets)
    - [Socket-like arguments](#socket-like-arguments)
- [Binding and local addresses](#binding-and-local-addresses)
- [Sending and receiving](#sending-and-receiving)
    - [Connected sockets](#connected-sockets)
    - [Unconnected sockets](#unconnected-sockets)
    - [Scatter-gather and ancillary data](#scatter-gather-and-ancillary-data)
- [Socket options](#socket-options)
    - [Low-level form](#low-level-form)
    - [Typed form](#typed-form)
    - [Option descriptors](#option-descriptors)
    - [Predefined Posix options](#predefined-posix-options)
    - [Predefined non-standard options](#predefined-non-standard-options)
    - [Predefined IPv4 and IPv6 options](#predefined-ipv4-and-ipv6-options)
    - [Boolean options](#boolean-options)
- [Notes on Windows](#notes-on-windows)

<!-- /TOC -->

## Overview

The `<ptl/socket.h>` header provides facilities for working with sockets. It contains:

- A `Socket` RAII wrapper that owns a socket and closes it when it goes out of scope. On Posix this is just an alias for `FileDescriptor`. On Windows it is a separate class that wraps a `SOCKET` handle.
- A `SocketLike` concept and a `c_socket` free function that let PTL methods accept raw socket handles, `Socket` objects, and your own socket-like types.
- Free functions wrapping the common socket-related calls (`socket`, `bind`, `getsockname`, `recv`, `send`, `recvfrom`, `sendto`, `recvmsg`, `sendmsg`, `setsockopt`, `getsockopt`).
- A `SockOptDesc` template and a collection of predefined option descriptors that bring compile-time type checking to socket options.

The header does not wrap `connect`, `listen`, `accept`, `shutdown`, or DNS resolution. These are obvious gaps and are likely to appear in future versions.

Everything is under `namespace ptl`. As with the rest of PTL, methods come in two forms: one that throws on failure and one that takes a trailing error code argument. See the [Error Handling](usage.md#error-handling) section of the usage guide for the general rules.

## Socket

### Lifecycle

On Posix `Socket` is a type alias for `FileDescriptor`. Sockets and file descriptors are the same thing there, so any file operation that takes a file-like argument also accepts a socket. See [file.md](file.md#filedescriptor) for the full lifecycle description.

On Windows `Socket` is a separate RAII class wrapping a `SOCKET` handle. It has the same shape as `FileDescriptor`: default construction, move-only semantics, `get`, `detach`, `close`, and `bool` conversion. Sockets and file descriptors are not interchangeable on Windows, so file operations do not accept a `Socket`.

```cpp
#include <ptl/socket.h>
using namespace ptl;

Socket empty;                  //owns nothing
Socket sock(rawHandle);        //takes ownership of an existing handle
```

The destructor calls `close` on Posix and `closesocket` on Windows. Conversion to `bool` is true when the object owns a valid socket.

### Creating sockets

The `createSocket` function wraps the `socket` system call. It takes the same domain, type, and protocol arguments and returns a new `Socket`.

```cpp
auto sock = createSocket(PF_INET, SOCK_STREAM, 0);   //TCP socket
auto udp  = createSocket(PF_INET, SOCK_DGRAM,  0);   //UDP socket
```

As usual, you can pass an error code object as the last argument:

```cpp
std::error_code ec;
auto sock = createSocket(PF_UNIX, SOCK_DGRAM, 0, ec);
if (ec) {
    //handle failure, sock is empty
}
```

### Socket-like arguments

PTL methods that operate on a socket accept anything that satisfies the `SocketLike` concept. On Posix this is the same as `FileDescriptorLike`, so the same set of types (raw `int`, `FileDescriptor`, `FILE *`, your own file-likes) works for sockets too. On Windows the concept requires a `SocketTraits` specialization; out of the box it covers raw `SOCKET` handles and `Socket` objects.

You can make your own type usable as a socket by specializing `SocketTraits` (on Windows) or `FileDescriptorTraits` (on Posix, which automatically makes it socket-like as well).

If you only want the raw handle, call `c_socket`:

```cpp
auto raw = c_socket(sock);
```

The return type is `int` on Posix and `SOCKET` on Windows.

## Binding and local addresses

`bindSocket` wraps `bind`. It takes a socket and a Posix-style `sockaddr` pointer with its length:

```cpp
sockaddr_in addr = {};
addr.sin_family = AF_INET;
addr.sin_port = 0;                          //let the kernel pick a port
addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

bindSocket(sock, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
```

`getSocketName` wraps `getsockname` and is the symmetric query:

```cpp
sockaddr_in addr = {};
ptl::socklen_t len = sizeof(addr);
getSocketName(sock, reinterpret_cast<sockaddr *>(&addr), &len);
//addr.sin_port now holds the actual port the kernel chose
```

Note the use of `ptl::socklen_t`. PTL aliases this to `::socklen_t` on Posix and to `int` on Windows, matching the Winsock convention. Use `ptl::socklen_t` in cross-platform code and you will not need to think about the difference.

The address structures themselves (`sockaddr`, `sockaddr_in`, `sockaddr_in6`, `sockaddr_un`, `in_addr`, `in6_addr`, and so on) are not wrapped by PTL. Use them as you would in plain Posix code.

## Sending and receiving

PTL provides `sendSocket` and `receiveSocket` as overloaded function names that cover the three Posix call shapes: the basic byte-buffer form, the form with an explicit peer address, and the message form with scatter-gather and ancillary data.

### Connected sockets

For a connected socket (one that has had `connect` called on it, or one obtained from `accept`), use the two-buffer form that wraps `send` and `recv`:

```cpp
char buf[1024];
auto received = receiveSocket(sock, buf, sizeof(buf), /*flags*/0);

auto sent = sendSocket(sock, "hello", 5, /*flags*/0);
```

The return type is `io_ssize_t`. On Posix this is `ssize_t`. On Windows it is `int`.

### Unconnected sockets

For unconnected sockets (typically datagram sockets), use the form that takes a peer address. This wraps `sendto` and `recvfrom`:

```cpp
sockaddr_in peer = {};
ptl::socklen_t peerLen = sizeof(peer);

//send a datagram to a specific address
auto sent = sendSocket(sock, "hello", 5, 0,
                       reinterpret_cast<const sockaddr *>(&peer), peerLen);

//receive a datagram and learn who sent it
auto received = receiveSocket(sock, buf, sizeof(buf), 0,
                              reinterpret_cast<sockaddr *>(&peer), &peerLen);
```

The length parameter for both `sendSocket` and `receiveSocket` is of type `io_size_t`. On platforms where the underlying call accepts a narrower count type (notably Windows, where `recv` and `send` take `int`), PTL checks for overflow at the wrapper boundary. Requesting a size larger than the underlying call can represent unconditionally throws `std::system_error` with `EINVAL`, since this kind of overflow is a logic bug rather than a runtime condition.

### Scatter-gather and ancillary data

For full control via `msghdr` you can pass a pointer to a `msghdr` structure. This wraps `recvmsg` and `sendmsg`:

```cpp
msghdr msg = {};
//set up msg.msg_iov, msg.msg_iovlen, msg.msg_control, etc.

auto received = receiveSocket(sock, &msg, /*flags*/0);
auto sent     = sendSocket(sock, &msg, /*flags*/0);
```

This form is Posix only. Windows has no equivalent of `msghdr`.

## Socket options

PTL exposes socket options at three levels of abstraction. The lowest level is a thin wrapper around `setsockopt` and `getsockopt`. On top of that is a templated form that handles size and type conversions automatically. On top of that is a type-checked form driven by predefined option descriptors.

In day-to-day code you will mostly use the third form and only reach for the lower forms when working with an option PTL does not yet know about.

### Low-level form

The lowest-level overloads of `setSocketOption` and `getSocketOption` mirror `setsockopt` and `getsockopt` directly:

```cpp
int yes = 1;
setSocketOption(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

int out;
ptl::socklen_t len = sizeof(out);
getSocketOption(sock, SOL_SOCKET, SO_REUSEADDR, &out, &len);
```

These exist mostly as building blocks for the higher-level forms, but you can use them directly if you need to pass an option that PTL does not have a descriptor for.

### Typed form

A templated overload takes the option value as a typed object and handles the size automatically:

```cpp
int yes = 1;
setSocketOption(sock, SOL_SOCKET, SO_REUSEADDR, yes);

auto out = getSocketOption<int>(sock, SOL_SOCKET, SO_REUSEADDR);
```

There is also a variant of the getter that takes the output by reference:

```cpp
int out;
getSocketOption(sock, SOL_SOCKET, SO_REUSEADDR, out);
```

The typed form has special handling for `bool` (see [Boolean options](#boolean-options)).

### Option descriptors

The most convenient form uses predefined `SockOptDesc` constants that bundle the level, the name, and the set of allowed value types. The compiler enforces that you pass a value of an allowed type:

```cpp
setSocketOption(sock, SockOptReuseAddr, true);             //bool is allowed
auto on = getSocketOption(sock, SockOptReuseAddr);         //returns bool

::linger linger { 1, 3 };
setSocketOption(sock, SockOptLinger, linger);              //::linger is allowed
::linger out;
getSocketOption(sock, SockOptLinger, out);
```

Some descriptors allow more than one type. For example `SockOptSndBuf` accepts both `unsigned` and `int`. You can pass either:

```cpp
setSocketOption(sock, SockOptSndBuf, 65536u);     //unsigned
setSocketOption(sock, SockOptSndBuf, 65536);      //int
```

When using the getter, the type is determined by the first type listed in the descriptor (unless you specify it explicitly):

```cpp
auto v = getSocketOption(sock, SockOptSndBuf);                //returns unsigned
auto v2 = getSocketOption<int>(sock, SockOptSndBuf);          //returns int
```

Passing the wrong type fails at compile time rather than at runtime:

```cpp
setSocketOption(sock, SockOptLinger, true);   //error: bool not allowed for SO_LINGER
```

You can build your own descriptors for options PTL does not predefine:

```cpp
constexpr auto SockOptCustom = SockOptDesc<int>{SOL_SOCKET, MY_SO_CUSTOM};

setSocketOption(sock, SockOptCustom, 42);
```

### Predefined Posix options

These are available on all platforms.

| Descriptor                | Allowed types              | Underlying option |
| ------------------------- | -------------------------- | ----------------- |
| `SockOptDebug`            | `bool`                     | `SO_DEBUG`        |
| `SockOptBroadcast`        | `bool`                     | `SO_BROADCAST`    |
| `SockOptReuseAddr`        | `bool`                     | `SO_REUSEADDR`    |
| `SockOptKeepAlive`        | `bool`                     | `SO_KEEPALIVE`    |
| `SockOptLinger`           | `::linger`                 | `SO_LINGER`       |
| `SockOptOOBInline`        | `bool`                     | `SO_OOBINLINE`    |
| `SockOptSndBuf`           | `unsigned`, `int`          | `SO_SNDBUF`       |
| `SockOptRcvBuf`           | `unsigned`, `int`          | `SO_RCVBUF`       |
| `SockOptDontRoute`        | `bool`                     | `SO_DONTROUTE`    |
| `SockOptRcvLowWatermark`  | `unsigned`, `int`          | `SO_RCVLOWAT`     |
| `SockOptRcvTimeout`       | `::timeval`                | `SO_RCVTIMEO`     |
| `SockOptSndLowWatermark`  | `unsigned`, `int`          | `SO_SNDLOWAT`     |
| `SockOptSndTimeout`       | `::timeval`                | `SO_SNDTIMEO`     |

### Predefined non-standard options

These are declared only when the underlying option is available on the target platform. Use `#ifdef` against the descriptor name (or against the underlying `SO_*` symbol) if you need to write portable code that uses them.

| Descriptor                     | Allowed types              | Underlying option         |
| ------------------------------ | -------------------------- | ------------------------- |
| `SockOptReusePort`             | `bool`                     | `SO_REUSEPORT`            |
| `SockOptType`                  | `int`                      | `SO_TYPE`                 |
| `SockOptError`                 | `int`                      | `SO_ERROR`                |
| `SockOptNoSIGPIPE`             | `bool`                     | `SO_NOSIGPIPE`            |
| `SockOptNRead`                 | `socklen_t`                | `SO_NREAD`                |
| `SockOptNWrite`                | `socklen_t`                | `SO_NWRITE`               |
| `SockOptLingerSec`             | `unsigned`, `int`          | `SO_LINGER_SEC`           |
| `SockOptAcceptsConn`           | `bool`                     | `SO_ACCEPTCONN`           |
| `SockOptTimestamp`             | `bool`                     | `SO_TIMESTAMP`            |
| `SockOptTimestampNs`           | `bool`                     | `SO_TIMESTAMPNS`          |
| `SockOptTimestampMonotonic`    | `bool`                     | `SO_TIMESTAMP_MONOTONIC`  |
| `SockOptDomain`                | `int`                      | `SO_DOMAIN`               |
| `SockOptProtocols`             | `int`                      | `SO_PROTOCOL`             |
| `SockOptBspState`              | `CSADDR_INFO`              | `SO_BSP_STATE`            |
| `SockOptExclusiveAddrUse`      | `bool`                     | `SO_EXCLUSIVEADDRUSE`     |

### Predefined IPv4 and IPv6 options

The IPv4 multicast interface and membership options accept `ip_mreqn` where available (Linux) and fall back to `ip_mreq` and `in_addr` on other platforms. PTL detects which structures are available at configuration time and adjusts the descriptor accordingly. You can pass any allowed type and the compiler will pick a match.

| Descriptor                       | Allowed types                                 | Underlying option         |
| -------------------------------- | --------------------------------------------- | ------------------------- |
| `SockOptIPv4MulticastLoop`       | `bool`                                        | `IP_MULTICAST_LOOP`       |
| `SockOptIPv4MulticastTtl`        | `uint8_t`, `int8_t`                           | `IP_MULTICAST_TTL`        |
| `SockOptIPv4MulticastIface`      | `ip_mreqn` (where supported), `ip_mreq`, `in_addr` | `IP_MULTICAST_IF`    |
| `SockOptIPv4AddMembership`       | `ip_mreqn` (where supported), `ip_mreq`       | `IP_ADD_MEMBERSHIP`       |
| `SockOptIPv4DropMembership`      | `ip_mreqn` (where supported), `ip_mreq`       | `IP_DROP_MEMBERSHIP`      |
| `SockOptIPv4MulticastAll`        | `bool`                                        | `IP_MULTICAST_ALL`        |
| `SockOptIPv6MulticastLoop`       | `bool`                                        | `IPV6_MULTICAST_LOOP`     |
| `SockOptIPv6MulticastHops`       | `int`                                         | `IPV6_MULTICAST_HOPS`     |
| `SockOptIPv6MulticastIface`      | `unsigned`, `int`                             | `IPV6_MULTICAST_IF`       |
| `SockOptIPv6MulticastAll`        | `bool`                                        | `IPV6_MULTICAST_ALL`      |

### Boolean options

Most kernels expect a boolean socket option as an `int`. A few platforms, notably OpenBSD and Solaris, expect a single byte for specific IPv4 multicast options. The typed `bool` overload of `setSocketOption` and `getSocketOption` papers over this:

- On set, PTL passes the value as `int` by default, and as `unsigned char` for the specific options that need it.
- On get, PTL accepts either width from the kernel and normalizes to `bool` for you.

The result is that you can write `setSocketOption(sock, SockOptIPv4MulticastLoop, true)` and the same code works on Linux, macOS, OpenBSD, Solaris, and Windows, even though they disagree about the wire format.

If you need bit-exact control, drop down to the low-level form and pass the value with your own buffer and length.

## Notes on Windows

Most of `<ptl/socket.h>` works on Windows via Winsock 2, but the surface is narrower.

What works:

- `Socket`, `createSocket`, `bindSocket`, `getSocketName`.
- `receiveSocket` and `sendSocket` in their byte-buffer and peer-address forms.
- `setSocketOption` and `getSocketOption` in all three forms.
- All predefined option descriptors whose underlying option is available on Windows.

What does not work:

- The `msghdr` overloads of `receiveSocket` and `sendSocket`. Winsock does not expose `recvmsg` or `sendmsg` in a compatible form.
- File operations on sockets. On Posix a socket is a file descriptor, so functions in `<ptl/file.h>` work on it. On Windows a `SOCKET` is not a file handle and these functions do not accept it.

A few platform details worth knowing:

- `ptl::socklen_t` is `int` on Windows. Use the alias in cross-platform code.
- Buffer pointers passed to Winsock are `char *`, not `void *`. PTL casts internally; you can pass `void *` to the PTL wrappers and they will do the right thing.
- The length argument to `recv` and `send` on Windows is `int`. The overflow check in `receiveSocket` and `sendSocket` throws when the requested length exceeds `INT_MAX`.
- The header `#define`s `NOMINMAX` before including `<winsock2.h>` to avoid clashes with the `min` and `max` macros from `<windows.h>`. It also explicitly `#undef`s them if they happen to be defined already. If you rely on those macros being defined in code that follows `<ptl/socket.h>`, this will affect you.
- Error reporting on Windows uses `WSAGetLastError` rather than `errno`. PTL handles this internally; the resulting `std::error_code` carries a Windows error category, and `errorEquals` from `<ptl/errors.h>` maps Windows error codes to `std::errc` values so you can write platform-independent comparisons.

You are responsible for calling `WSAStartup` before using any Winsock functionality and `WSACleanup` at the end. PTL does not call these on your behalf.
