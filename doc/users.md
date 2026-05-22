# User Information

<!--
 Notes to AI grammar checkers:
   - this document uses Posix in preference to POSIX.
   - this document does not require pedantic comma after e.g.
-->

<!-- TOC depthfrom:2 -->

- [Overview](#overview)
- [Passwd](#passwd)
    - [Looking up users](#looking-up-users)
    - [Reading fields](#reading-fields)
    - [Lifetime of string pointers](#lifetime-of-string-pointers)
- [Group](#group)
- [Notes on Windows and Android](#notes-on-windows-and-android)

<!-- /TOC -->

## Overview

The `<ptl/users.h>` header provides facilities for looking up entries in the system's user and group databases. It wraps the reentrant Posix calls:

- `getpwnam_r` and `getpwuid_r`, exposed through the `Passwd` class.
- `getgrnam_r` and `getgrgid_r`, exposed through the `Group` class.

Both classes have the same shape and share an implementation. They are RAII objects that own the lookup result and the backing buffer that the underlying C call needs.

Everything is under `namespace ptl`. The functions in this header support both throwing and error code forms. See the [Error Handling](usage.md#error-handling) section of the usage guide for the general rules.

The whole header is Posix only.

## Passwd

`Passwd` represents a user record. It is implemented as a class that inherits publicly from `struct passwd`, so the standard fields (`pw_name`, `pw_uid`, `pw_gid`, `pw_dir`, `pw_shell`, and so on) are accessible directly.

### Looking up users

Looking up a user by login name or numeric user id is done through static factory methods that return `std::optional<Passwd>`. The result is empty when the user is not found, populated when the user exists, and the call throws (or sets the error code) on actual errors:

```cpp
#include <ptl/users.h>
using namespace ptl;

auto entry = Passwd::getByName("alice");
if (entry) {
    //alice exists
} else {
    //alice does not exist; this is not an error
}

auto entry2 = Passwd::getById(1000);
if (entry2) {
    //a user with uid 1000 exists
}
```

As usual, you can pass an error code as the trailing argument:

```cpp
std::error_code ec;
auto entry = Passwd::getByName("alice", ec);
if (ec) {
    //a real error occurred (e.g. permission, I/O)
} else if (!entry) {
    //alice does not exist
}
```

The name argument can be any string-like type: a string literal, `std::string`, `std::string_view`, and so on, as long as the underlying string is null-terminated.

### Reading fields

After a successful lookup the standard `struct passwd` fields are accessible directly:

```cpp
auto entry = Passwd::getByName("alice");
if (entry) {
    const char * home  = entry->pw_dir;
    const char * shell = entry->pw_shell;
    uid_t uid          = entry->pw_uid;
    gid_t gid          = entry->pw_gid;
    //...
}
```

Field availability depends on the platform; consult the Posix `struct passwd` documentation for the portable members and your platform's manuals for extensions.

### Lifetime of string pointers

The pointer fields in `struct passwd` (`pw_name`, `pw_dir`, `pw_shell`, and the others) point into a buffer owned by the `Passwd` object. When the `Passwd` is destroyed the buffer is freed and these pointers become dangling.

```cpp
const char * shell;
{
    auto entry = Passwd::getByName("alice");
    shell = entry->pw_shell;          //pointer into the entry's buffer
}                                     //entry destroyed here, buffer freed
//shell is now a dangling pointer; do not dereference
```

If you need the data to outlive the `Passwd` object, copy it into a `std::string` (or whatever else makes sense):

```cpp
std::string shell;
{
    auto entry = Passwd::getByName("alice");
    if (entry)
        shell = entry->pw_shell;
}
//shell is independent of the entry now
```

`Passwd` is move-only. Moving transfers ownership of the buffer, so the pointer fields remain valid in the destination.

## Group

`Group` represents a group record. It has the same shape as `Passwd` and the same lifetime rules apply to its pointer fields.

```cpp
auto entry = Group::getByName("wheel");
if (entry) {
    gid_t gid = entry->gr_gid;
    //gr_mem is a null-terminated array of pointers to member names
    for (char ** p = entry->gr_mem; *p; ++p) {
        const char * member = *p;
        //...
    }
}

auto entry2 = Group::getById(0);   //look up gid 0
```

`Group` is only available on Android API level 24 and above, since the underlying reentrant calls were added then.

## Notes on Windows and Android

The whole header is unavailable on Windows. The Windows security model has no equivalent of `/etc/passwd` or `/etc/group`; use the Win32 user and group APIs directly if you need that information.

On Android the situation is more nuanced:

- `Passwd` is available on all supported Android API levels.
- `Group` requires API level 24 or above.
- On Android, `getpwnam_r` and `getpwuid_r` return `ENOENT` when the user is not found, whereas Posix says they should return 0 with a null result pointer. PTL papers over this difference: `getByName` and `getById` return an empty `optional` for not-found regardless of platform. You will not see the deviation.
