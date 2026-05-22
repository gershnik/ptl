# PTL process facilities

<!--
 Notes to AI grammar checkers:
   - this document uses Posix in preference to POSIX.
   - this document does not require pedantic comma after e.g.
-->

<!-- TOC depthfrom:2 -->

- [Overview](#overview)
- [ChildProcess](#childprocess)
    - [Obtaining a ChildProcess](#obtaining-a-childprocess)
    - [Waiting for a child](#waiting-for-a-child)
    - [Non-blocking and untraced waits](#non-blocking-and-untraced-waits)
    - [Detaching ownership](#detaching-ownership)
    - [Destruction](#destruction)
- [Process-like arguments](#process-like-arguments)
- [Sessions and process groups](#sessions-and-process-groups)
- [Notes on Windows](#notes-on-windows)

<!-- /TOC -->

## Overview

The `<ptl/process.h>` header provides facilities to represent and manage child processes. It contains:

- An RAII wrapper, `ChildProcess`, that owns a child process and waits for it when it goes out of scope.
- A `ProcessLike` concept and a `c_pid` free function that let other PTL methods accept either a raw `pid_t` or a `ChildProcess` (and, with a small traits specialization, your own process-like types).
- Wrappers for `setsid` and `setpgid` on Posix.

This header is deliberately small. It does not contain anything that creates a child process. To launch one, see `<ptl/spawn.h>` which exposes `forkProcess`, `spawn` and `exec`. To send signals to a child, see `<ptl/signal.h>`. The types and the concept defined here are used by both.

Everything is under `namespace ptl`. As with the rest of PTL, methods come in two forms: one that throws on failure and one that takes a trailing error code argument. See the [Error Handling](usage.md#error-handling) section of the usage guide for the general rules.

## ChildProcess

`ChildProcess` is an RAII wrapper over a child process identifier. It can be empty (no child) or it can own a running or terminated-but-not-yet-reaped child.

```cpp
#include <ptl/process.h>
using namespace ptl;

ChildProcess none;            // empty
ChildProcess proc(somePid);   // takes ownership of an existing child
```

The default-constructed object is empty. The `pid_t` constructor is explicit, takes ownership of the supplied identifier and asserts that it is non-negative. Conversion to `bool` is true when the object owns a child.

```cpp
if (proc) {
    //we own a child
}
```

`ChildProcess` is move-only. Moving leaves the source empty.

### Obtaining a ChildProcess

In typical use you will not construct a `ChildProcess` from a raw `pid_t`. Instead you will get one back from a function in `<ptl/spawn.h>`:

```cpp
#include <ptl/spawn.h>
using namespace ptl;

auto proc = spawn({"/bin/sh", "-c", "echo hello"});
//proc is a ChildProcess owning the new process
```

The raw `pid_t` constructor exists for the cases where you have obtained an identifier by other means and want to attach RAII semantics to it.

### Waiting for a child

The `wait` method wraps `waitpid`. In its simplest form it blocks until the child changes state and returns the raw status as `std::optional<int>`. As usual, you can pass an error code as the trailing argument:

```cpp
auto stat = proc.wait();          //throws on failure
//or
std::error_code ec;
auto stat = proc.wait(ec);        //sets ec on failure
```

When the child has terminated, `wait` returns the status wrapped in an `optional` and leaves the `ChildProcess` empty. You can then test the status with the standard Posix macros:

```cpp
auto stat = proc.wait().value();
if (WIFEXITED(stat)) {
    int code = WEXITSTATUS(stat);
    //child exited normally with code
} else if (WIFSIGNALED(stat)) {
    int sig = WTERMSIG(stat);
    //child was killed by signal sig
}
```

Calling `wait` on an empty `ChildProcess` is a programmer error and unconditionally throws `std::system_error` with `EINVAL`. This is consistent with the rest of PTL: calls that can only fail due to logic errors do not offer the error code form.

### Non-blocking and untraced waits

The two-argument form takes a `flags` parameter that is passed straight through to `waitpid`. The flags you are most likely to use are `WNOHANG` and `WUNTRACED`.

`WNOHANG` makes the call return immediately even if the child has not changed state. In that case the returned optional is empty:

```cpp
//poll
if (auto stat = proc.wait(WNOHANG); stat) {
    //child has terminated, *stat is the status
} else {
    //child is still running, proc still owns it
}
```

`WUNTRACED` causes `wait` to also report stopped children. A stopped child is not gone, so the `ChildProcess` remains non-empty and you can wait again:

```cpp
auto stat = proc.wait(WUNTRACED).value();
if (WIFSTOPPED(stat)) {
    //child stopped, proc is still valid
    //we may wait on it again later
}
```

The rule is straightforward: ownership ends when the child is gone, which on Posix means `WIFEXITED(stat) || WIFSIGNALED(stat)`. Until then the `ChildProcess` continues to own the identifier and `bool` conversion stays true.

### Detaching ownership

If you want to take the raw identifier out of a `ChildProcess` and disable its automatic wait, use `detach`:

```cpp
pid_t raw = proc.detach();
//proc is now empty, raw is the previous pid
//the caller is now responsible for reaping the child
```

After `detach` the object behaves as if default-constructed. This is useful when you need to hand the identifier to another component or when you genuinely want the child to outlive the scope (the daemon pattern).

The `get` method returns the current identifier without giving up ownership:

```cpp
pid_t raw = proc.get();   //proc still owns the child
```

### Destruction

The destructor of a non-empty `ChildProcess` waits for the child to terminate. This mirrors the contract of `std::jthread` and exists for the same reason: on Posix every child must eventually be waited for, otherwise it stays around as a zombie. RAII is the natural place to enforce this.

The practical consequences:

```cpp
{
    auto proc = spawn({"/bin/sleep", "60"});
    //...
}  //blocks here for up to 60 seconds
```

If you do not want this behavior, either call `wait` explicitly before the object goes out of scope, or call `detach` and arrange for the child to be reaped elsewhere (for example by a `SIGCHLD` handler or by `signal(SIGCHLD, SIG_IGN)` which on Linux causes children to be reaped automatically).

The destructor retries on `EINTR` so a signal arriving during the wait does not cause the child to be leaked. It does not throw under any circumstances.

## Process-like arguments

PTL methods that operate on a process do not require a `ChildProcess`. They accept anything that satisfies the `ProcessLike` concept, which means anything whose process identifier can be extracted via `ProcessTraits`. Out of the box this covers `pid_t` and `ChildProcess`:

```cpp
#include <ptl/process.h>
#include <ptl/signal.h>
using namespace ptl;

ChildProcess proc = spawn({"/bin/sleep", "60"});

sendSignal(proc, SIGTERM);          //works with ChildProcess
sendSignal(proc.get(), SIGTERM);    //works with raw pid_t
sendSignal(::getpid(), SIGUSR1);    //works with any pid_t value
```

You can extract the identifier yourself with `c_pid` if you need it:

```cpp
pid_t p = c_pid(proc);   //same as proc.get()
pid_t p2 = c_pid(1234);  //identity
```

You can also make your own type work as a process by specializing `ProcessTraits`:

```cpp
class MyProcess { ... };

template<> struct ptl::ProcessTraits<MyProcess> {
    static pid_t c_pid(const MyProcess & p) noexcept { return p.id(); }
};

//now MyProcess can be passed wherever ProcessLike is accepted
```

This lets you mix PTL with code that already has its own process abstractions.

## Sessions and process groups

Two thin wrappers are provided for the Posix session and process group calls. Both are unavailable on Windows.

`setSessionId` wraps `setsid` and creates a new session with the calling process as the leader. It returns the new session identifier and, as usual, supports both error modes:

```cpp
pid_t sid = setSessionId();        //throws on failure
//or
std::error_code ec;
pid_t sid = setSessionId(ec);
```

`setProcessGroupId` wraps `setpgid`. It takes any process-like object and the desired group identifier:

```cpp
ChildProcess proc = spawn({"/bin/sleep", "60"});

setProcessGroupId(proc, 0);          //put the child into its own group
setProcessGroupId(::getpid(), 0);    //put ourselves into our own group
```

Passing `0` for the group identifier means "use the process's own pid as the group id", which is the usual way to create a new group with the target process as its leader.

## Notes on Windows

On Windows PTL still defines `pid_t`. On MinGW it is the system `pid_t`. On the rest of Windows it is `intptr_t`, which matches the type returned by `_spawn` family functions and accepted by `_cwait`.

`ChildProcess` is available on Windows. Its destructor waits via `_cwait` rather than `waitpid`. The `flags` parameter of `wait` must be `0` on Windows (this is checked by an assert in the implementation). The status returned by `wait` on Windows is the exit code reported by `_cwait`, not a Posix-style packed status, so the `WIFEXITED` and `WEXITSTATUS` macros do not apply. On Windows, treat the integer as the raw exit code.

`setSessionId` and `setProcessGroupId` are not provided on Windows since the underlying calls do not exist.

The `ProcessLike` concept and the `c_pid` accessor work identically on all platforms.
