# PTL usage guide

<!--
 Notes to AI grammar checkers:
   - this document uses Posix in preference to POSIX.
   - this document does not require pedantic comma after e.g.
-->

<!-- TOC depthfrom:2 -->

- [Basics](#basics)
- [Naming](#naming)
- [Error handling](#error-handling)
    - [Throwing form](#throwing-form)
    - [Error code form](#error-code-form)
    - [Whitelisting expected errors](#whitelisting-expected-errors)
    - [Calls that always throw](#calls-that-always-throw)
    - [No std::expected or outcome](#no-stdexpected-or-outcome)
- [Path arguments](#path-arguments)
- [Per-header guides](#per-header-guides)

<!-- /TOC -->

## Basics

To use PTL, either include individual `ptl/foo.h` headers (see [Function Mapping](function-mapping.md) for which header provides which functionality) or include the umbrella header `ptl/ptl.h`.

Everything in the library is under `namespace ptl`.

The library targets C++20. It is header only. It depends only on the standard library, except that if your standard library is missing `std::format` it will use `{fmt}` instead.

## Naming

PTL uses `CamelCase` for types and `lowerUpper` for methods. It also mostly avoids method names that would be identical to raw Posix calls. This is done for a reason. Two reasons, actually.

1. Some Posix calls can actually be implemented as macros (on _some platforms_, to make it worse). You can imagine the resulting havoc if a `foo` method name in PTL clashed with a macro. There are ways around it inside the library, but the problem usually leaks to users too.
2. Even without macros, you should be able to say `using namespace ptl` and then call PTL methods without the `ptl::` prefix, and not have to deal with "ambiguous call" errors when names and argument types overlap with raw Posix calls. Or worse, have the raw call invoked when you thought you called the safe wrapper. Using a different convention and slightly more elaborate names (e.g. `duplicate` for `dup`, `makeDirectory` for `mkdir`) avoids this entirely.


## Error handling

PTL uses a strategy similar to (but not identical with) the one popularized by `std::filesystem`, and `boost::filesystem` before it.

### Throwing form

Most methods that can fail can be called in two ways. The first form throws on failure:

```cpp
#include <ptl/file.h>
using namespace ptl;

try {
    auto fd = FileDescriptor::open("test_file", O_WRONLY | O_CREAT,
                                   S_IRWXU | S_IRWXG | S_IRWXO);
    changeMode(fd, S_IRUSR | S_IWUSR);
} catch (std::system_error & ex) {
    //handle failure
}
```

By default the thrown type is `std::system_error`. This can be globally overridden with another exception type or replaced with program termination if you do not use exceptions in your codebase.

### Error code form

The second form returns the error in an output parameter:

```cpp
#include <ptl/file.h>
using namespace ptl;

bool foo(std::error_code & ec) {
    auto fd = FileDescriptor::open("test_file", O_WRONLY | O_CREAT,
                                   S_IRWXU | S_IRWXG | S_IRWXO, ec);
    if (ec)
        return false;

    changeMode(fd, S_IRUSR | S_IWUSR, ec);
    if (ec)
        return false;

    return true;
}
```

The parameter is cleared on success, set on failure. Its type defaults to `std::error_code` but can be `boost::error_code` or your own error type by specializing some traits. Unlike the typical `std::filesystem` implementation, this is done without code duplication via some template machinery.

### Whitelisting expected errors

When using exceptions, sometimes you want certain errors to be returned rather than thrown because they are _expected_. The classic example is opening a file that might or might not be there. The `AllowedErrors` template in `<ptl/errors.h>` lets you do this cleanly:

```cpp
#include <ptl/errors.h>
#include <ptl/file.h>
using namespace ptl;

try {
    AllowedErrors<ENOENT, EACCES> ec;
    auto fd = FileDescriptor::open("maybe_nonexistent", O_RDONLY, ec);
    if (ec) {
        //either ENOENT or EACCES happened
        //any other error would have thrown
    }
} catch (std::system_error & ex) {
    //something bad and unexpected
}
```

### Calls that always throw

Some PTL calls are throw-only and do not offer the error code form. These are calls that can only legitimately fail due to a logic error (e.g. `EINVAL`, "you passed me an invalid argument") or due to truly catastrophic conditions ("the world is falling apart"). The thinking is that even in exception-free code the only reasonable response to such failures is to terminate, so there is no point in increasing client complexity.

`duplicate` and `duplicateTo` are examples of throw-only calls. Overflow precondition checks at wrapper boundaries (such as in `readFile` and `writeFile` on platforms where the underlying count type is narrower than `size_t`) also always throw.

### No std::expected or outcome

PTL does not use, and does not plan to use, `std::expected` or `outcome`. This is partly because the first is not yet commonly available. More importantly, the `outcome` style unconditionally penalizes people who want to use exceptions: they pay for what they do not use both in mental burden (do not forget to dereference that outcome) and in generated code (whether the compiler can always see through outcome manipulations and optimize them out is unclear).

## Path arguments

Any PTL method that takes a path accepts a variety of types: `std::filesystem::path`, `std::string`, `const char *`, and more. You can make your own types acceptable by specializing the `CPathTraits` template in `<ptl/core.h>`.

```cpp
std::string filename_str = "some_file";
auto fd1 = FileDescriptor::open(filename_str, O_RDONLY);

std::filesystem::path filename_path = "some_file";
auto fd2 = FileDescriptor::open(filename_path, O_RDONLY);

makeDirectory("some_dir", S_IRWXU | S_IRWXG);
makeDirectory(std::filesystem::path("/some/dir"), S_IRWXU | S_IRWXG);
```

Note that `std::string_view` is not accepted, because the type carries no guarantee of being null-terminated and PTL refuses to copy silently.

## Per-header guides

Detailed coverage of each major header is split into its own document:

- [file.md](file.md): `<ptl/file.h>`. `FileDescriptor` objects, reading and writing, locking, mode and ownership, pipes, memory maps, directory operations.
- [process.md](process.md): `<ptl/process.h>`. The `ChildProcess` RAII wrapper, waiting for children, sessions and process groups.
- [spawn.md](spawn.md): `<ptl/spawn.h>`. Creating child processes via `forkProcess`, the `spawn` family, and the `exec` family.
- [socket.md](socket.md): `<ptl/socket.h>`. The `Socket` wrapper, sending and receiving, type-checked socket options.
- [signal.md](signal.md): `<ptl/signal.h>`. The `SignalSet` and `SignalAction` classes, sending and raising signals, installing handlers, process signal mask.
- [identity.md](identity.md): `<ptl/identity.h>`. Setting real and effective user and group ids, managing supplementary groups.
- [users.md](users.md): `<ptl/users.h>`. Looking up entries in the user and group databases.
- [system.md](system.md): `<ptl/system.h>`. System configuration queries and host name.

For an exhaustive list of which Posix call is wrapped by which PTL function, see [function-mapping.md](function-mapping.md).

