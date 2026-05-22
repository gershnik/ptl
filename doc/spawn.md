# Creating Processes

<!--
 Notes to AI grammar checkers:
   - this document uses Posix in preference to POSIX.
   - this document does not require pedantic comma after e.g.
-->

<!-- TOC depthfrom:2 -->

- [Overview](#overview)
- [Choosing between spawn, fork+exec and exec alone](#choosing-between-spawn-forkexec-and-exec-alone)
- [forkProcess](#forkprocess)
- [spawn](#spawn)
    - [Argument arrays](#argument-arrays)
    - [SpawnFileActions](#spawnfileactions)
    - [SpawnAttr](#spawnattr)
    - [SpawnSettings](#spawnsettings)
    - [Searching PATH](#searching-path)
- [exec and execp](#exec-and-execp)
- [Android compatibility](#android-compatibility)
- [Notes on Windows](#notes-on-windows)

<!-- /TOC -->

## Overview

The `<ptl/spawn.h>` header provides facilities to create child processes. It wraps three families of Posix calls:

- `fork`, exposed as `forkProcess`.
- The `exec` family, exposed as `exec` (no PATH search) and `execp` (with PATH search).
- `posix_spawn` and `posix_spawnp`, exposed as the `spawn` function family along with the helper classes `SpawnFileActions`, `SpawnAttr` and `SpawnSettings`.

All of these return or interact with the `ChildProcess` RAII wrapper defined in `<ptl/process.h>`. See [process.md](process.md) for details on `ChildProcess`.

Everything is under `namespace ptl`. As elsewhere in PTL, methods come in throwing and error-code forms; see the [Error Handling](usage.md#error-handling) section of the usage guide for the general rules.

## Choosing between spawn, fork+exec and exec alone

PTL exposes both `posix_spawn`-style and `fork`+`exec`-style process creation because they have different trade-offs:

- `spawn` is the recommended path for the common case of "launch this program and let it run". It is implemented atomically by the kernel where possible, can be much cheaper than `fork`+`exec` on systems with copy-on-write overhead, and works on platforms that do not support `fork` at all (including modern Android above API 28).
- `fork`+`exec` is the right choice when you need to run arbitrary code in the child between forking and exec-ing. The classic example is a shell setting up redirections, signal masks and process groups in ways that go beyond what `posix_spawn` can express.
- `exec` alone is for replacing the current process with a different executable without creating a child at all. This is what a child does after `fork` in the `fork`+`exec` pattern.

If you find yourself reaching for `fork`+`exec` only to do file descriptor redirections or signal mask adjustments, look at `SpawnFileActions` and `SpawnAttr` first. They cover most of what shell-style setup requires.

## forkProcess

`forkProcess` is a thin wrapper over `fork`. It returns a `ChildProcess`.

```cpp
#include <ptl/spawn.h>
using namespace ptl;

auto childProc = forkProcess();
if (childProc) {
    //parent: childProc owns the new child
} else {
    //child: childProc is empty
    //do work and either exec or exit
}
```

The semantics match `fork`. In the parent, on success, the returned `ChildProcess` owns the new child and is truthy. In the child, the returned `ChildProcess` is empty and falsy. On failure (in the parent only, since failure can only happen there), the call either throws or sets the error code parameter, and the returned object is empty.

When using the error code form, you must check the error code to distinguish "I am in the child" from "fork failed in the parent". Both cases give you an empty `ChildProcess`, but only one of them also sets the error:

```cpp
std::error_code ec;
auto childProc = forkProcess(ec);
if (ec) {
    //fork failed in the parent
} else if (childProc) {
    //parent, childProc owns the child
} else {
    //child
}
```

`forkProcess` is not available on MinGW since Windows has no `fork`.

A classic use of `forkProcess` is to set up a pipe and then arrange the child to feed its stdout through it:

```cpp
auto [readEnd, writeEnd] = Pipe::create();

auto childProc = forkProcess();
if (childProc) {
    writeEnd.close();
    //read from readEnd, then wait for the child
    auto stat = childProc.wait().value();
} else {
    readEnd.close();
    duplicateTo(writeEnd, stdout);
    execp({"sh", "-c", "echo hello"});
}
```

## spawn

The `spawn` family wraps `posix_spawn` and `posix_spawnp` on Posix and `_spawnve` / `_spawnvpe` on Windows. All overloads return a `ChildProcess`.

In the simplest form you only need an argument array. The executable to run is taken from the first element:

```cpp
auto proc = spawn({"/bin/sh", "-c", "echo hello"});
```

If you want the executable path to differ from `argv[0]`, pass it as a separate first argument:

```cpp
auto proc = spawn("/bin/sh", {"sh", "-c", "echo hello"});
```

You can also supply an environment (otherwise the child inherits the current one) and a `SpawnSettings` object that controls file actions and other attributes. There are overloads for every combination of these. The full form is:

```cpp
auto proc = spawn(exe, args, env, settings);
```

and you can drop any of `exe`, `env` or `settings`. The remaining `args` is always required.

As usual, you can pass an error code object as the last argument:

```cpp
std::error_code ec;
auto proc = spawn({"some_program"}, ec);
if (ec) {
    //handle failure
}
```

### Argument arrays

The `args` and `env` parameters have type `StringRefArray`. You can pass a braced initializer list of string literals (the common case), a `std::vector<std::string>`, a `std::array` of `const char *`, or any other range of string-like elements. PTL handles the conversion internally and avoids copying when the input is already a null-terminated array of `const char *`.

```cpp
//all of these work
spawn({"/bin/ls", "-l", "/tmp"});

std::vector<std::string> argv = {"/bin/ls", "-l", "/tmp"};
spawn(argv);

std::array<const char *, 3> arr = {"/bin/ls", "-l", "/tmp"};
spawn(arr);
```

### SpawnFileActions

`SpawnFileActions` wraps `posix_spawn_file_actions_t`. It holds a list of file descriptor operations to apply in the child before the program is executed. The supported operations are the standard Posix three plus a few common extensions where available.

```cpp
#include <ptl/spawn.h>
using namespace ptl;

SpawnFileActions actions;

//close fd 5 in the child
actions.addClose(5);

//open /dev/null as the child's stdout
actions.addOpen(stdout, "/dev/null", O_WRONLY, 0);

//redirect the child's stdout into a pipe
auto [readEnd, writeEnd] = Pipe::create();
actions.addDuplicateTo(writeEnd, stdout);
```

`addClose`, `addOpen` and `addDuplicateTo` correspond to `posix_spawn_file_actions_addclose`, `posix_spawn_file_actions_addopen` and `posix_spawn_file_actions_adddup2`. As elsewhere in PTL, file descriptor arguments accept anything that satisfies the `FileDescriptorLike` concept (raw `int`, `FILE *`, `FileDescriptor`, and so on) and path arguments accept anything that satisfies `PathLike`.

The following extension methods are available on platforms that support the corresponding underlying calls. PTL detects support at configuration time and the methods are only declared when usable.

- `addInheritNp` wraps `posix_spawn_file_actions_addinherit_np`. Marks a descriptor as inheritable.
- `addCloseFromNp` wraps `posix_spawn_file_actions_addclosefrom_np`. Closes all descriptors at or above a given number.
- `addChdirNp` wraps `posix_spawn_file_actions_addchdir_np`. Changes the working directory in the child.

`SpawnFileActions` is a move-only RAII object. Its constructor calls `posix_spawn_file_actions_init` and can throw on allocation failure. Its destructor calls `posix_spawn_file_actions_destroy`.

This class is Posix only. It is not available on Windows.

### SpawnAttr

`SpawnAttr` wraps `posix_spawnattr_t`. It controls process-level attributes such as signal handling and process group placement.

```cpp
SpawnAttr attr;

//reset all signals to their default disposition in the child
attr.setFlags(POSIX_SPAWN_SETSIGDEF);
attr.setSigDefault(SignalSet::all());

//put the child into a new process group
attr.setFlags(POSIX_SPAWN_SETPGROUP);
attr.setPGroup(0);
```

The methods are:

- `setFlags` wraps `posix_spawnattr_setflags`. Use the `POSIX_SPAWN_*` constants from `<spawn.h>`. Flags control which other settings are honored.
- `setSigDefault` wraps `posix_spawnattr_setsigdefault`. Takes a `SignalSet` from `<ptl/signal.h>` (see [Signals](signal.md)). Only effective if `POSIX_SPAWN_SETSIGDEF` is set in the flags.
- `setPGroup` wraps `posix_spawnattr_setpgroup`. Only effective if `POSIX_SPAWN_SETPGROUP` is set in the flags. Passing `0` (the default) means "use the child's own pid as the group id", which creates a new process group with the child as the leader.

Like `SpawnFileActions`, `SpawnAttr` is a move-only RAII object that can throw on construction and is Posix only.

### SpawnSettings

`SpawnSettings` is a small builder that bundles file actions, attributes and the choice of PATH search into a single argument for `spawn`. It is cheap to construct and copy.

```cpp
SpawnFileActions actions;
actions.addOpen(stdout, "/dev/null", O_WRONLY, 0);

SpawnAttr attr;
attr.setFlags(POSIX_SPAWN_SETSIGDEF);
attr.setSigDefault(SignalSet::all());

auto proc = spawn({"/bin/sh", "-c", "ls"},
                  SpawnSettings().fileActions(actions).attr(attr));
```

The `fileActions` and `attr` methods take a reference and store a pointer to it inside the settings object. The referenced `SpawnFileActions` or `SpawnAttr` must outlive the `spawn` call. To make this safer, the rvalue overloads are explicitly deleted, so the following will not compile:

```cpp
//error: temporary would dangle
auto proc = spawn({"/bin/sh", "-c", "ls"},
                  SpawnSettings().fileActions(SpawnFileActions()));
```

You can also pass raw `posix_spawn_file_actions_t *` and `posix_spawnattr_t *` pointers to `fileActions` and `attr` if you have prepared them by hand. The settings object does not take ownership.

`SpawnSettings` is available on Windows but only exposes the methods that make sense there, which is essentially `usePath()`. On Windows there is no `posix_spawn_file_actions_t` or `posix_spawnattr_t` to wrap.

### Searching PATH

By default `spawn` requires a full or relative path to the executable. To search the user's `PATH`, call `usePath()` on the settings object:

```cpp
auto proc = spawn({"sh", "-c", "echo hello"},
                  SpawnSettings().usePath());
```

This switches the underlying call from `posix_spawn` to `posix_spawnp` (or from `_spawnve` to `_spawnvpe` on Windows).

## exec and execp

`exec` and `execp` replace the current process with a different executable. They wrap the `execve`/`execv` and `execvpe`/`execvp` Posix calls respectively. They never return on success: when they do return, it is because the call failed, and PTL converts that into either an exception or a populated error code.

```cpp
#include <ptl/spawn.h>
using namespace ptl;

//replace this process with /bin/ls
exec("/bin/ls", {"ls", "-l"});

//or, search PATH
execp({"ls", "-l"});
```

The argument shapes match `spawn`. You can pass the executable path separately or let PTL take it from `args[0]`. You can pass an explicit environment or let the child inherit the current one. Each combination has its own overload.

Since these functions only return on failure, the return type is `void`. Using the error code form is straightforward:

```cpp
std::error_code ec;
exec("/no/such/program", {"prog"}, ec);
//if we reach this line, exec failed and ec is set
```

The path-searching form `execp` is available only when the platform provides the corresponding underlying call. The version with both PATH search and an explicit environment, `execvpe`, is a glibc extension and not portable. PTL detects support at configuration time, so the four-argument form of `execp` may not exist on all platforms. The path-search form without an environment override is portable.

`exec` and `execp` are Posix only. Windows does not provide them in a useful form.

## Android compatibility

The `spawn` family and the `SpawnFileActions` and `SpawnAttr` helpers require Android API level 28 or higher, since `posix_spawn` was added to Bionic in that release. PTL conditionally compiles them out on older API levels and only declares them when usable.

`forkProcess`, `exec` and `execp` are available on all supported Android API levels.

Be aware that even where the calls exist, Bionic occasionally deviates from strict Posix semantics. For example, `posix_spawn` on Android does not report an error when the executable does not exist, since the path is resolved in the child after fork and the parent has no way to learn of the failure. PTL cannot paper over this; if you need that error reported, wait on the child and observe how it terminates.

## Notes on Windows

The Windows surface is intentionally narrow.

`forkProcess` is not available on MinGW. Windows has no `fork`, and emulating it is outside the scope of this library.

`spawn` is available and maps to `_spawnve` (or `_spawnvpe` when `usePath()` is set). The returned `ChildProcess` owns a Windows process handle (or rather, the identifier accepted by `_cwait`). `SpawnFileActions` and `SpawnAttr` are not available on Windows; if you need to control file descriptors or process attributes for the new process, you will need to use the Windows API directly. `SpawnSettings` still exists but only `usePath()` does anything useful.

`exec` and `execp` are not available on Windows. The Windows `_execve` family does exist but its semantics differ enough that PTL does not wrap it. If you need to replace the current process on Windows, call it directly.
