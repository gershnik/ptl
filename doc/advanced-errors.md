# Advanced Error Handling

<!--
 Notes to AI grammar checkers:
   - this document uses Posix in preference to POSIX.
   - this document does not require pedantic comma after e.g.
-->

<!-- TOC depthfrom:2 -->

- [Overview](#overview)
- [Portable error comparison with errorEquals](#portable-error-comparison-with-errorequals)
- [Custom error types](#custom-error-types)

<!-- /TOC -->

## Overview

This document covers two topics in `<ptl/errors.h>` that go beyond the basic throwing-or-error-code dichotomy described in the [main usage guide](usage.md#error-handling):

- `errorEquals`, a helper for comparing a `std::error_code` against an `std::errc` value portably and efficiently.
- `ErrorTraits`, the customization point that lets you plug your own error type into any PTL call that takes an error code.

Both facilities live in `<ptl/errors.h>` and are available on every supported platform.

## Portable error comparison with errorEquals

When working with `std::error_code` in cross-platform code, the obvious thing to write is:

```cpp
if (ec == std::errc::no_such_file_or_directory) {
    //handle missing file
}
```

This works correctly on Posix because Posix errors are stored in `std::system_category` (or `std::generic_category`) with values that match `std::errc` directly. The comparison short-circuits to an integer compare.

On Windows the situation is messier. PTL stores Windows errors in `std::system_category` with raw Win32 values (`ERROR_FILE_NOT_FOUND = 2`, `WSAECONNREFUSED = 10061`, and so on). The standard library does provide a mapping from these to `std::errc` via `default_error_condition`, but the mapping has historically been incomplete on some compilers, especially for Winsock codes. The comparison may also be slower than necessary because it goes through virtual category calls.

`errorEquals` solves both problems:

```cpp
#include <ptl/errors.h>
using namespace ptl;

std::error_code ec;
auto fd = FileDescriptor::open("missing", O_RDONLY, ec);
if (errorEquals(ec, std::errc::no_such_file_or_directory)) {
    //works portably and quickly on every platform
}
```

The function inspects the error category and:

- For `std::generic_category` and Posix `std::system_category`, compares the raw value directly.
- For Windows `std::system_category`, consults PTL's own Windows-to-Posix mapping table. This table covers both classic Win32 errors and the Winsock `WSAE*` family.
- For anything else (a user-defined category, for instance), falls back to the standard `ec == code` comparison.

The signature is straightforward:

```cpp
auto errorEquals(const std::error_code & ec, std::errc code) -> bool;
```

Note that `errorEquals` is asymmetric. The first argument is whatever PTL (or anything else) produced; the second is the abstract `std::errc` value you are testing for. There is no overload taking two `std::errc` values or two `std::error_code` values because neither would have anything useful to add over a direct comparison.

## Custom error types

The error code form is not limited to `std::error_code`. Any type that satisfies the `ErrorSink` concept can be passed as the trailing error argument, which lets you plug in `boost::system::error_code`, a custom error-with-context class, or anything else that fits your project's conventions.

A type satisfies `ErrorSink` when `ErrorTraits<T>` is specialized in the `ptl` namespace to provide three static operations:

- `assignError(T & dest, Error err, const char * format, args...)`: put `dest` into a state representing the given error. The format string and arguments describe what went wrong; a simple trait may ignore them, a richer one can fold them into the stored error (for example, to capture an explanatory message).
- `clearError(T & dest) noexcept`: put `dest` into the success state.
- `failed(const T & src) noexcept -> bool`: return whether `src` represents a failure.

The `Error` type passed to `assignError` is a small struct holding a numeric error code. On Windows it also carries a tag indicating whether the code is Posix or Win32. It is the raw form PTL produces internally before adapting to whatever target the caller chose.

Out of the box, traits are provided for `std::error_code`, `ptl::Error` (which stores the raw error directly), and `ptl::AllowedErrors<...>` (the whitelist helper described in the [usage guide](usage.md#whitelisting-expected-errors)).

To support your own type, specialize the trait. As an example, here is what enabling `boost::system::error_code` looks like:

```cpp
namespace ptl {
    template<> struct ErrorTraits<boost::system::error_code> {
        template<class... T>
        static void assignError(boost::system::error_code & dest, Error err,
                                const char *, T && ...) noexcept {
            #ifndef _WIN32
                dest.assign(err.code, boost::system::system_category());
            #else
                dest.assign(err.code,
                    err.type == Error::Posix
                        ? boost::system::generic_category()
                        : boost::system::system_category());
            #endif
        }
        static void clearError(boost::system::error_code & err) noexcept {
            err.clear();
        }
        static bool failed(const boost::system::error_code & err) noexcept {
            return bool(err);
        }
    };
}
```

After this specialization, every PTL call that takes an error parameter accepts a `boost::system::error_code` in the same way it accepts a `std::error_code`. No PTL code is recompiled and no surface area changes.

A trait that wants to capture more than just the error code can use the format string and arguments. For example, a trait for a custom logging error type might construct a formatted message via `std::format` (or `fmt::format`) and store it alongside the code.
