# PTL signal facilities

<!--
 Notes to AI grammar checkers:
   - this document uses Posix in preference to POSIX.
   - this document does not require pedantic comma after e.g.
-->

<!-- TOC depthfrom:2 -->

- [Overview](#overview)
- [SignalSet](#signalset)
- [Signal names and messages](#signal-names-and-messages)
- [Sending signals](#sending-signals)
- [Simple signal handlers](#simple-signal-handlers)
- [Rich signal handling with sigaction](#rich-signal-handling-with-sigaction)
- [Process signal mask](#process-signal-mask)
- [Notes on Windows](#notes-on-windows)

<!-- /TOC -->

## Overview

The `<ptl/signal.h>` header provides facilities for working with Posix signals. It contains:

- The `SignalSet` class wrapping `sigset_t`.
- `signalName` and `signalMessage` for converting a signal number to a human-readable string.
- `sendSignal` and `raiseSignal` wrapping `kill` and `raise`.
- `setSignalHandler` wrapping the simple `signal` interface.
- The `SignalAction` class and the `setSignalAction` / `getSignalAction` functions wrapping `sigaction`.
- `setSignalProcessMask` and `getSignalProcessMask` wrapping `sigprocmask`.

Everything is under `namespace ptl`. As elsewhere in PTL, methods come in throwing and error code forms; see the [Error Handling](usage.md#error-handling) section of the usage guide for the general rules. The `sigaction` and `sigprocmask` wrappers are throw-only since their possible failures are all logic errors.

## SignalSet

`SignalSet` is a thin wrapper over `sigset_t`. Its default constructor produces an empty set; explicit constructors and named factories let you obtain the common starting points:

```cpp
#include <ptl/signal.h>
using namespace ptl;

SignalSet empty;                 //empty
SignalSet all = SignalSet::all();
SignalSet none = SignalSet::none();

sigset_t raw = ...;
SignalSet fromRaw(raw);          //wrap an existing sigset_t
```

You can add, remove, and query individual signals:

```cpp
SignalSet set;
set.add(SIGINT);
set.add(SIGTERM);

if (set.isMember(SIGINT)) {
    //yes, it is
}

set.del(SIGINT);
```

The underlying `sigset_t` is accessible through `get`, in both const and non-const overloads, for code that needs to call Posix functions directly:

```cpp
sigset_t * raw = &set.get();
//pass raw to some Posix function that takes sigset_t *
```

`SignalSet` is Posix only. It is not declared on Windows.

## Signal names and messages

`signalName` returns a short uppercase mnemonic for a signal (e.g. `"INT"` for `SIGINT`, `"KILL"` for `SIGKILL`). It is available on every platform. When PTL cannot produce a symbolic name (because none of the platform's name tables knows the signal, or because no name tables exist at all), `signalName` falls back to the signal number rendered as a decimal string. This means it always returns something printable.

```cpp
std::string s = signalName(SIGINT);   //"INT" on most platforms, "2" otherwise
```

`signalName` consults `sigabbrev_np` (glibc), `sys_signame` (BSDs and macOS), and `sys_sigabbrev` (older glibc and Cygwin) in that order of preference, depending on what PTL detected at configuration time.

`signalMessage` wraps `strsignal` and returns a longer description ("Interrupt", "Killed", and so on). It is Posix only.

```cpp
std::string s = signalMessage(SIGINT);   //"Interrupt"
```

Note that `strsignal` is not thread-safe on all platforms. If you need a thread-safe string in cross-platform code, prefer `signalName`.

## Sending signals

`sendSignal` wraps `kill`. It takes anything that satisfies the `ProcessLike` concept from `<ptl/process.h>`, which means raw `pid_t`, a `ChildProcess`, or your own process-like types:

```cpp
#include <ptl/process.h>
#include <ptl/signal.h>
using namespace ptl;

sendSignal(::getpid(), SIGUSR1);          //send to ourselves by pid

auto child = spawn({"/bin/sleep", "60"});
sendSignal(child, SIGTERM);                //send to a ChildProcess
```

This function is not available on MinGW, since Windows has no `kill`.

`raiseSignal` wraps `raise` and is portable. It raises the signal in the current process:

```cpp
raiseSignal(SIGUSR1);
```

Sending a signal can fail when the target process does not exist or when the caller lacks permission. As usual, you can pass an error code as the trailing argument.

## Simple signal handlers

The simple `signal` call is wrapped by `setSignalHandler`. It installs a handler for a single signal and returns the previous handler. It is throw-only since the failure modes (invalid signal number, attempting to set a handler for `SIGKILL` or `SIGSTOP`) are all logic errors.

```cpp
auto previous = setSignalHandler(SIGINT, [](int sig) {
    //handle Ctrl+C
});

//later, restore the previous handler
setSignalHandler(SIGINT, previous);

//or restore the default
setSignalHandler(SIGINT, SIG_DFL);
```

The simple interface is appropriate for casual use and for the few cases where you want maximum portability. For anything serious, use `setSignalAction`.

## Rich signal handling with sigaction

`SignalAction` wraps `struct sigaction`. It inherits publicly from the underlying C struct, which means you can access the `sa_handler`, `sa_sigaction`, `sa_flags`, and `sa_mask` members directly and pass a `SignalAction *` anywhere a `struct sigaction *` is expected. The class adds constructors and a `setMask` helper that make the common cases tidy.

The four constructors cover the typical combinations of handler type and flags:

```cpp
//plain handler, no extra flags
SignalAction a1([](int sig) { /*...*/ });

//siginfo handler, sets SA_SIGINFO automatically
SignalAction a2([](int sig, siginfo_t * si, void * ctx) { /*...*/ });

//plain handler with extra flags
SignalAction a3([](int sig) { /*...*/ }, SA_RESTART);

//siginfo handler with extra flags
SignalAction a4([](int sig, siginfo_t * si, void * ctx) { /*...*/ },
                SA_RESTART | SA_NODEFER);
```

The `setMask` method assigns the blocked-signals mask from a `SignalSet`:

```cpp
SignalSet block;
block.add(SIGUSR2);

SignalAction action([](int){});
action.setMask(block);
```

To install an action use `setSignalAction`. The two-argument form discards the previous action; the three-argument form returns it through the output parameter:

```cpp
//discard previous action
setSignalAction(SIGUSR1, SignalAction([](int sig){
    //handle SIGUSR1
}));

//capture previous action
SignalAction previous;
setSignalAction(SIGUSR1, SignalAction([](int sig){ /*...*/ }), previous);
```

To query the current action without changing it use `getSignalAction`:

```cpp
SignalAction current;
getSignalAction(SIGUSR1, current);
```

These functions are throw-only and Posix only.

## Process signal mask

The process-wide blocked signal mask is controlled with `setSignalProcessMask`, which wraps `sigprocmask`. The `how` argument is one of `SIG_BLOCK`, `SIG_UNBLOCK`, or `SIG_SETMASK`.

The two-argument form discards the previous mask; the three-argument form returns it:

```cpp
SignalSet block;
block.add(SIGINT);

setSignalProcessMask(SIG_BLOCK, block);

SignalSet previous;
setSignalProcessMask(SIG_BLOCK, block, previous);
```

To query the current mask without changing it use `getSignalProcessMask`:

```cpp
SignalSet current;
getSignalProcessMask(current);
```

These functions are throw-only and Posix only.

## Notes on Windows

The Windows surface of `<ptl/signal.h>` is narrow because the C runtime's signal model is very limited.

What works on Windows:

- `signalName`. Always available, falls back to a numeric string when no name table is present.
- `setSignalHandler`. Uses the underlying CRT `signal`.
- `raiseSignal`. Uses the underlying CRT `raise`.

What does not work on Windows:

- `SignalSet`, `SignalAction`, `setSignalAction`, `getSignalAction`. There is no `sigaction` on Windows.
- `setSignalProcessMask`, `getSignalProcessMask`. There is no `sigprocmask` on Windows.
- `signalMessage`. There is no `strsignal` on Windows.
- `sendSignal`. There is no `kill` on Windows. PTL declines to emulate it.

On MinGW the situation is slightly different from MSVC: MinGW still does not provide most of the Posix signal API, but it does provide some headers that look like they do. PTL relies on what is actually callable rather than what is declared, so the available surface is the same as on MSVC.
