# Functionality supported by PTL

<!-- Links -->

[file.h]:       ../inc/ptl/file.h
[identity.h]:   ../inc/ptl/identity.h
[process.h]:    ../inc/ptl/process.h
[signal.h]:     ../inc/ptl/signal.h
[spawn.h]:      ../inc/ptl/spawn.h
[system.h]:     ../inc/ptl/system.h
[users.h]:      ../inc/ptl/users.h

[chdir()]:          https://pubs.opengroup.org/onlinepubs/9699919799/functions/chdir.html
[chmod()]:          https://pubs.opengroup.org/onlinepubs/9699919799/functions/chmod.html
[chown()]:          https://pubs.opengroup.org/onlinepubs/9699919799/functions/chown.html
[chroot()]:         https://pubs.opengroup.org/onlinepubs/7908799/xsh/chroot.html
[close()]:          https://pubs.opengroup.org/onlinepubs/9699919799/functions/close.html
[dup()]:            https://pubs.opengroup.org/onlinepubs/9699919799/functions/dup.html
[dup2()]:           https://pubs.opengroup.org/onlinepubs/9699919799/functions/dup2.html
[fchdir()]:         https://pubs.opengroup.org/onlinepubs/9699919799/functions/fchdir.html
[fchmod()]:         https://pubs.opengroup.org/onlinepubs/9699919799/functions/fchmod.html
[fchown()]:         https://pubs.opengroup.org/onlinepubs/9699919799/functions/fchown.html
[fork()]:           https://pubs.opengroup.org/onlinepubs/9699919799/functions/fork.html
[fstat()]:          https://pubs.opengroup.org/onlinepubs/9699919799/functions/fstat.html
[ftruncate()]:      https://pubs.opengroup.org/onlinepubs/9699919799/functions/ftruncate.html
[kill()]:           https://pubs.opengroup.org/onlinepubs/9699919799/functions/kill.html
[lchown()]:         https://pubs.opengroup.org/onlinepubs/9699919799/functions/lchown.html
[lstat()]:          https://pubs.opengroup.org/onlinepubs/9699919799/functions/lstat.html
[mkdir()]:          https://pubs.opengroup.org/onlinepubs/9699919799/functions/mkdir.html
[mkdirat()]:        https://pubs.opengroup.org/onlinepubs/9699919799/functions/mkdirat.html
[open()]:           https://pubs.opengroup.org/onlinepubs/9699919799/functions/open.html
[pipe()]:           https://pubs.opengroup.org/onlinepubs/9699919799/functions/pipe.html
[raise()]:          https://pubs.opengroup.org/onlinepubs/9699919799/functions/raise.html
[read()]:           https://pubs.opengroup.org/onlinepubs/9699919799/functions/read.html
[setgid()]:         https://pubs.opengroup.org/onlinepubs/9699919799/functions/setgid.html
[setegid()]:        https://pubs.opengroup.org/onlinepubs/9699919799/functions/setegid.html
[seteuid()]:        https://pubs.opengroup.org/onlinepubs/9699919799/functions/seteuid.html
[setpgid()]:        https://pubs.opengroup.org/onlinepubs/9699919799/functions/setpgid.html
[setsid()]:         https://pubs.opengroup.org/onlinepubs/9699919799/functions/setsid.html
[setuid()]:         https://pubs.opengroup.org/onlinepubs/9699919799/functions/setuid.html
[sigaction()]:      https://pubs.opengroup.org/onlinepubs/9699919799/functions/sigaction.html
[sigaddset()]:      https://pubs.opengroup.org/onlinepubs/9699919799/functions/sigaddset.html
[sigdelset()]:      https://pubs.opengroup.org/onlinepubs/9699919799/functions/sigdelset.html
[sigemptyset()]:    https://pubs.opengroup.org/onlinepubs/9699919799/functions/sigemptyset.html
[sigfillset()]:     https://pubs.opengroup.org/onlinepubs/9699919799/functions/sigfillset.html
[sigismember()]:    https://pubs.opengroup.org/onlinepubs/9699919799/functions/sigismember.html
[signal()]:         https://pubs.opengroup.org/onlinepubs/9699919799/functions/signal.html
[sigprocmask()]:    https://pubs.opengroup.org/onlinepubs/9699919799/functions/sigprocmask.html
[stat()]:           https://pubs.opengroup.org/onlinepubs/9699919799/functions/stat.html
[strsignal()]:      https://pubs.opengroup.org/onlinepubs/9699919799/functions/strsignal.html
[truncate()]:       https://pubs.opengroup.org/onlinepubs/9699919799/functions/truncate.html
[waitpid()]:        https://pubs.opengroup.org/onlinepubs/9699919799/functions/waitpid.html
[write()]:          https://pubs.opengroup.org/onlinepubs/9699919799/functions/write.html

[flock-lin]:        https://man7.org/linux/man-pages/man2/flock.2.html
[mkostemps-lin]:    https://man7.org/linux/man-pages/man3/mkstemp.3.html
[sigabbrev_np()]:   https://man7.org/linux/man-pages/man3/sigabbrev_np.3.html

[flock-mac]:        https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/flock.2.html
[lchmod-mac]:       https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man3/lchmod.3.html
[mkostemps-mac]:    https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man3/mkstemp.3.html

[flock-bsd]:        https://man.freebsd.org/cgi/man.cgi?query=flock
[lchmod-bsd]:       https://man.freebsd.org/cgi/man.cgi?query=lchmod
[mkostemps-bsd]:    https://man.freebsd.org/cgi/man.cgi?query=mkostemp
[sys_signame]:      https://man.freebsd.org/cgi/man.cgi?query=sys_signame

<!-- Links -->


| Name           | Exposed by                   | Header       | Availability
|----------------|------------------------------|--------------|--------------
|[chdir()]       | `changeDirectory()`          | [file.h]     | 
|[chmod()]       | `changeMode()`               | [file.h]     | 
|[chown()]       | `changeOwner()`              | [file.h]     | 
|[chroot()]      | `changeRoot()`               | [file.h]     | Removed from Posix but universally available
|[close()]       | `FileDescriptor::~FileDescriptor()`, `FileDescriptor::close()` | [file.h] | 
|[dup()]         | `duplicate()`                | [file.h]     | 
|[dup2()]        | `duplicateTo()`              | [file.h]     | 
|[fchdir()]      | `changeDirectory()`          | [file.h]     | 
|[fchmod()]      | `changeMode()`               | [file.h]     | 
|[fchown()]      | `changeOwner()`              | [file.h]     | 
|`flock()`       | `lockFile()`, `tryLockFile()`, `unlockFile()` | [file.h] | [Linux][flock-lin], [Mac][flock-mac], [BSD][flock-bsd]
|[fork()]        | `forkProcess()`              | [spawn.h]    |
|[fstat()]       | `getStatus()`                | [file.h]     |
|[ftruncate()]   | `truncateFile()`             | [file.h]     |
|[kill()]        | `sendSignal()`               | [signal.h]   | 
|`lchmod()`      | `changeLinkMode()`           | [file.h]     | [Mac][mkostemps-mac], [BSD][mkostemps-bsd]
|[lchown()]      | `changeLinkOwner()`          | [file.h]     | 
|[lstat()]       | `getLinkStatus()`            | [file.h]     | 
|`mkostemps()`   | `FileDescriptor::openTemp()` | [file.h]     | [Linux][mkostemps-lin], [Mac][mkostemps-mac], [BSD][mkostemps-bsd]
|[mkdir()]       | `makeDirectory()`            | [file.h]     | 
|[mkdirat()]     | `makeDirectoryAt()`          | [file.h]     | 
|[open()]        | `FileDescriptor::open()`     | [file.h]     | 
|[pipe()]        | `Pipe::create()`             | [file.h]     | 
|[raise()]       | `raiseSignal()`              | [signal.h]   | 
|[read()]        | `readFile()`                 | [file.h]     | 
|[setgid()]      | `setGid()`                   | [identity.h] |
|[setegid()]     | `setEffectiveGid()`          | [identity.h] |
|[seteuid()]     | `setEffectiveUid()`          | [identity.h] |
|[setpgid()]     | `setProcessGroupId()`        | [process.h]  |
|[setsid()]      | `setSessionId()`             | [process.h]  |
|[setuid()]      | `setUid()`                   | [identity.h] |
| [sigabbrev_np()], [sys_signame], `sys_sigabbrev` | `signalName()` | [signal.h] | Various non-portable ways of obtaining a signal name. The `signalName()` is supported on all platforms and falls back on returning signal number converted to string if no known mapping is available
|[sigaction()]   | `setSignalAction()`, `getSignalAction()` | [signal.h] | Note that `struct sigaction` is minimally wrapped by `SignalAction` class
|[sigaddset()]   | `SignalSet::add()`           | [signal.h]   |
|[sigdelset()]   | `SignalSet::del()`           | [signal.h]   |
|[sigemptyset()] | `SignalSet::SignalSet()`, `SignalSet::none()` | [signal.h] |
|[sigfillset()]  | `SignalSet::all()`           | [signal.h]   |
|[sigismember()] | `SignalSet::isMember()`      | [signal.h]   |
|[signal()]      | `setSignalHandler()`         | [signal.h]   |
|[sigprocmask()] | `setSignalProcessMask()`, `getSignalProcessMask()`| [signal.h] |
|[stat()]        | `getStatus()`                | [file.h]     | 
|[strsignal()]   | `signalMessage()`            | [signal.h]   | 
|[truncate()]    | `truncateFile()`             | [file.h]     |
|[waitpid()]     | `ChildProcess::~ChildProcess()`, `ChildProcess::wait()` | [process.h] | 
|[write()]       | `writeFile()`                | [file.h]     | 
