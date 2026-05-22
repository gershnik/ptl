# PTL system facilities

<!--
 Notes to AI grammar checkers:
   - this document uses Posix in preference to POSIX.
   - this document does not require pedantic comma after e.g.
-->

<!-- TOC depthfrom:2 -->

- [Overview](#overview)
- [systemConfig](#systemconfig)
- [getHostName](#gethostname)
- [Notes on Windows](#notes-on-windows)

<!-- /TOC -->

## Overview

The `<ptl/system.h>` header is the catch-all for "give me some system-wide information" calls that do not naturally belong with file, process, signal, or identity facilities. As such, its membership grows on demand: it currently wraps `sysconf` and `gethostname`, and other small system-info calls may be added over time.

Everything is under `namespace ptl`. The header is Posix only.

## systemConfig

`systemConfig` wraps `sysconf`. Posix specifies a long list of configurable system values (open file limits, page size, host name maximum length, and so on) that can be queried by passing the appropriate `_SC_*` constant. The wrapper returns `std::optional<long>`:

```cpp
#include <ptl/system.h>
using namespace ptl;

auto openMax = systemConfig(_SC_OPEN_MAX);     //typical: file descriptor limit
auto pageSize = systemConfig(_SC_PAGE_SIZE);
auto hostNameMax = systemConfig(_SC_HOST_NAME_MAX);
```

The returned `optional` is:

- Populated with a long when the system has a definite value for the queried constant.
- Empty when the value is "no limit" or "not defined" on this system. This is distinct from an error and is the normal way Posix signals "unlimited".

The call is throw-only. It throws `std::system_error` with `EINVAL` if the `_SC_*` constant is not recognized on the current platform. Since this is a logic error (you passed something the system does not know about), no error code form is offered. A typical pattern is to use `value_or` to supply a sensible default:

```cpp
auto max = size_t(systemConfig(_SC_HOST_NAME_MAX).value_or(_POSIX_HOST_NAME_MAX));
```

This reads as "ask the system for its host name limit; if it has no concrete number, fall back to the Posix minimum".

## getHostName

`getHostName` wraps `gethostname` and fills the supplied buffer with the host name.

```cpp
std::string buf(256, '\0');
getHostName(buf);
buf.resize(std::strlen(buf.c_str()));
```

The buffer is passed as a `std::span<char>`. The exact size to allocate depends on the platform; on most systems the host name fits comfortably in 256 bytes, but for a portable upper bound query the limit through `systemConfig(_SC_HOST_NAME_MAX)` first.

```cpp
auto len = size_t(systemConfig(_SC_HOST_NAME_MAX).value_or(_POSIX_HOST_NAME_MAX));
std::string buf(len + 1, '\0');
getHostName(buf);
buf.resize(std::strlen(buf.c_str()));
```

On success, the buffer is guaranteed to be null-terminated. This is the function's main safety contribution over the raw call: some platforms (notably Android) do not null-terminate `gethostname`'s output when the host name is too long to fit, leaving the caller with a buffer that looks like a string but is not. PTL writes a null byte to the final position of the buffer on success, so reading the result as a C string is always safe.

The side effect of this guarantee is that a host name exactly the size of the buffer loses its last character. If you want the full host name regardless of length, allocate `len + 1` bytes (as in the example above) and the truncation never bites.

As usual, the function supports both error modes. On failure (typically `ENAMETOOLONG` when the buffer is too small) it throws or sets the error code:

```cpp
std::string buf(1, '\0');
std::error_code ec;
getHostName(buf, ec);
if (ec) {
    //either ENAMETOOLONG or some other failure
}
```

## Notes on Windows

The entire header is unavailable on Windows. `sysconf` does not exist there at all, and while Winsock provides its own `gethostname`, it has different semantics and lives in a different header. If you need either kind of information on Windows, use the Win32 APIs directly.
