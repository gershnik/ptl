# User Identity

<!--
 Notes to AI grammar checkers:
   - this document uses Posix in preference to POSIX.
   - this document does not require pedantic comma after e.g.
-->

<!-- TOC depthfrom:2 -->

- [Overview](#overview)
- [Real and effective user and group ids](#real-and-effective-user-and-group-ids)
- [Supplementary groups](#supplementary-groups)
- [Notes on Windows](#notes-on-windows)

<!-- /TOC -->

## Overview

The `<ptl/identity.h>` header provides facilities for managing the calling process's user and group identity. It wraps:

- `setuid`, `setgid`, `seteuid`, and `setegid` as `setUid`, `setGid`, `setEffectiveUid`, and `setEffectiveGid`.
- `getgroups` as `getGroups`.
- `setgroups` as `setGroups`, where the platform provides it.

Reading the current identity is not wrapped here. Use `::getuid`, `::geteuid`, `::getgid`, and `::getegid` directly; they take no arguments, never fail in any meaningful sense, and adding a thin wrapper would not improve anything.

Everything is under `namespace ptl`. The functions in this header support both throwing and error code forms. See the [Error Handling](usage.md#error-handling) section of the usage guide for the general rules.

The whole header is Posix only. None of these calls have meaningful counterparts on Windows.

## Real and effective user and group ids

These four functions wrap their Posix namesakes one for one. They are the canonical way for a privileged process to drop privileges.

```cpp
#include <ptl/identity.h>
using namespace ptl;

setGid(1);           //set the real (and effective and saved) group id
setUid(1);           //set the real (and effective and saved) user id

setEffectiveGid(1);  //set only the effective group id
setEffectiveUid(1);  //set only the effective user id
```

The typical privilege-dropping sequence is to drop the group id first, then the user id. Once you have dropped the user id you may no longer have the permission to drop the group id, so the order matters.

```cpp
setGroups({/* the supplementary groups you want to keep */});
setGid(targetGid);
setUid(targetUid);
//we are now running as targetUid:targetGid with the given supplementary groups
```

As usual, all four functions support both error modes:

```cpp
std::error_code ec;
setUid(1, ec);
if (ec) {
    //typical failure: EPERM, you were not root
}
```

## Supplementary groups

`getGroups` returns the calling process's supplementary group list:

```cpp
std::vector<gid_t> groups = getGroups();
```

`setGroups` replaces the supplementary group list. It takes either a `std::span<const gid_t>` or an initializer list:

```cpp
setGroups({1, 2, 3});

std::vector<gid_t> wanted = {/* ... */};
setGroups(wanted);
```

If your supplied list is too large to fit into the count type the platform's `setgroups` accepts (`size_t` on Linux glibc, `int` on most BSDs), `setGroups` unconditionally throws `std::system_error` with `EINVAL`, since this kind of overflow is a logic bug rather than a runtime condition.

`setGroups` is only declared when PTL detects `setgroups` at configuration time. Some platforms (notably some Android API levels) lack it entirely.

A privileged process that wants to drop both its primary identity and its supplementary group list can do so like this:

```cpp
setGroups({newGid});       //or some explicit list
setGid(newGid);
setUid(newUid);
```

## Notes on Windows

The entire header is unavailable on Windows. The Windows security model is based on access tokens rather than numeric user and group ids, and there is no useful analog to `setuid` or `setgroups`. If you need to change identity on Windows, use the Win32 token APIs directly.
