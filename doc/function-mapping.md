# Functionality supported by PTL

<!-- Links -->

[file.h]:       ../inc/ptl/file.h
[identity.h]:   ../inc/ptl/identity.h
[process.h]:    ../inc/ptl/process.h
[signal.h]:     ../inc/ptl/signal.h
[spawn.h]:      ../inc/ptl/spawn.h
[socket.h]:     ../inc/ptl/socket.h
[system.h]:     ../inc/ptl/system.h
[users.h]:      ../inc/ptl/users.h

[bind()]:           https://pubs.opengroup.org/onlinepubs/9699919799/functions/bind.html
[chdir()]:          https://pubs.opengroup.org/onlinepubs/9699919799/functions/chdir.html
[chmod()]:          https://pubs.opengroup.org/onlinepubs/9699919799/functions/chmod.html
[chown()]:          https://pubs.opengroup.org/onlinepubs/9699919799/functions/chown.html
[chroot()]:         https://pubs.opengroup.org/onlinepubs/7908799/xsh/chroot.html
[close()]:          https://pubs.opengroup.org/onlinepubs/9699919799/functions/close.html
[dup()]:            https://pubs.opengroup.org/onlinepubs/9699919799/functions/dup.html
[dup2()]:           https://pubs.opengroup.org/onlinepubs/9699919799/functions/dup2.html
[exec()]:           https://pubs.opengroup.org/onlinepubs/9699919799/functions/exec.html
[fchdir()]:         https://pubs.opengroup.org/onlinepubs/9699919799/functions/fchdir.html
[fchmod()]:         https://pubs.opengroup.org/onlinepubs/9699919799/functions/fchmod.html
[fchown()]:         https://pubs.opengroup.org/onlinepubs/9699919799/functions/fchown.html
[fork()]:           https://pubs.opengroup.org/onlinepubs/9699919799/functions/fork.html
[fstat()]:          https://pubs.opengroup.org/onlinepubs/9699919799/functions/fstat.html
[ftruncate()]:      https://pubs.opengroup.org/onlinepubs/9699919799/functions/ftruncate.html
[getgrnam_r()]:     https://pubs.opengroup.org/onlinepubs/9699919799/functions/getgrnam_r.html
[getgroups()]:      https://pubs.opengroup.org/onlinepubs/9699919799/functions/getgroups.html
[getgruid_r()]:     https://pubs.opengroup.org/onlinepubs/9699919799/functions/getgruid_r.html
[gethostname()]:    https://pubs.opengroup.org/onlinepubs/9699919799/functions/gethostname.html
[getpwnam_r()]:     https://pubs.opengroup.org/onlinepubs/9699919799/functions/getpwnam_r.html
[getpwuid_r()]:     https://pubs.opengroup.org/onlinepubs/9699919799/functions/getpwuid_r.html
[getsockname()]:    https://pubs.opengroup.org/onlinepubs/9699919799/functions/getsockname.html
[getsockopt()]:     https://pubs.opengroup.org/onlinepubs/9699919799/functions/getsockopt.html
[kill()]:           https://pubs.opengroup.org/onlinepubs/9699919799/functions/kill.html
[lchown()]:         https://pubs.opengroup.org/onlinepubs/9699919799/functions/lchown.html
[lstat()]:          https://pubs.opengroup.org/onlinepubs/9699919799/functions/lstat.html
[mkdir()]:          https://pubs.opengroup.org/onlinepubs/9699919799/functions/mkdir.html
[mkdirat()]:        https://pubs.opengroup.org/onlinepubs/9699919799/functions/mkdirat.html
[mmap()]:           https://pubs.opengroup.org/onlinepubs/9699919799/functions/mmap.html
[munmap()]:         https://pubs.opengroup.org/onlinepubs/9699919799/functions/munmap.html
[open()]:           https://pubs.opengroup.org/onlinepubs/9699919799/functions/open.html
[pipe()]:           https://pubs.opengroup.org/onlinepubs/9699919799/functions/pipe.html
[posix_spawn_file_actions_addclose()]:  https://pubs.opengroup.org/onlinepubs/9699919799/functions/posix_spawn_file_actions_addclose.html
[posix_spawn_file_actions_adddup2()]:   https://pubs.opengroup.org/onlinepubs/9699919799/functions/posix_spawn_file_actions_adddup2.html
[posix_spawn_file_actions_addopen()]:   https://pubs.opengroup.org/onlinepubs/9699919799/functions/posix_spawn_file_actions_addopen.html
[posix_spawn_file_actions_destroy()]:   https://pubs.opengroup.org/onlinepubs/9699919799/functions/posix_spawn_file_actions_destroy.html
[posix_spawn_file_actions_init()]:      https://pubs.opengroup.org/onlinepubs/9699919799/functions/posix_spawn_file_actions_init.html
[posix_spawnattr_destroy()]:            https://pubs.opengroup.org/onlinepubs/9699919799/functions/posix_spawnattr_destroy.html
[posix_spawnattr_init()]:               https://pubs.opengroup.org/onlinepubs/9699919799/functions/posix_spawnattr_init.html
[posix_spawnattr_setflags()]:           https://pubs.opengroup.org/onlinepubs/9699919799/functions/posix_spawnattr_setflags.html
[posix_spawnattr_setsigdefault()]:      https://pubs.opengroup.org/onlinepubs/9699919799/functions/posix_spawnattr_setsigdefault.html
[posix_spawnattr_setpgroup()]:          https://pubs.opengroup.org/onlinepubs/9699919799/functions/posix_spawnattr_setpgroup.html
[posix_spawn()]:    https://pubs.opengroup.org/onlinepubs/9699919799/functions/posix_spawn.html
[posix_spawnp()]:   https://pubs.opengroup.org/onlinepubs/9699919799/functions/posix_spawnp.html
[raise()]:          https://pubs.opengroup.org/onlinepubs/9699919799/functions/raise.html
[read()]:           https://pubs.opengroup.org/onlinepubs/9699919799/functions/read.html
[recv()]:           https://pubs.opengroup.org/onlinepubs/9699919799/functions/recv.html
[recvfrom()]:       https://pubs.opengroup.org/onlinepubs/9699919799/functions/recvfrom.html
[recvmsg()]:        https://pubs.opengroup.org/onlinepubs/9699919799/functions/recvmsg.html
[send()]:           https://pubs.opengroup.org/onlinepubs/9699919799/functions/send.html
[sendto()]:         https://pubs.opengroup.org/onlinepubs/9699919799/functions/sendto.html
[sendmsg()]:        https://pubs.opengroup.org/onlinepubs/9699919799/functions/sendmsg.html
[setgid()]:         https://pubs.opengroup.org/onlinepubs/9699919799/functions/setgid.html
[setegid()]:        https://pubs.opengroup.org/onlinepubs/9699919799/functions/setegid.html
[seteuid()]:        https://pubs.opengroup.org/onlinepubs/9699919799/functions/seteuid.html
[setpgid()]:        https://pubs.opengroup.org/onlinepubs/9699919799/functions/setpgid.html
[setsid()]:         https://pubs.opengroup.org/onlinepubs/9699919799/functions/setsid.html
[setsockopt()]:     https://pubs.opengroup.org/onlinepubs/9699919799/functions/setsockopt.html
[setuid()]:         https://pubs.opengroup.org/onlinepubs/9699919799/functions/setuid.html
[sigaction()]:      https://pubs.opengroup.org/onlinepubs/9699919799/functions/sigaction.html
[sigaddset()]:      https://pubs.opengroup.org/onlinepubs/9699919799/functions/sigaddset.html
[sigdelset()]:      https://pubs.opengroup.org/onlinepubs/9699919799/functions/sigdelset.html
[sigemptyset()]:    https://pubs.opengroup.org/onlinepubs/9699919799/functions/sigemptyset.html
[sigfillset()]:     https://pubs.opengroup.org/onlinepubs/9699919799/functions/sigfillset.html
[sigismember()]:    https://pubs.opengroup.org/onlinepubs/9699919799/functions/sigismember.html
[signal()]:         https://pubs.opengroup.org/onlinepubs/9699919799/functions/signal.html
[sigprocmask()]:    https://pubs.opengroup.org/onlinepubs/9699919799/functions/sigprocmask.html
[socket()]:         https://pubs.opengroup.org/onlinepubs/9699919799/functions/socket.html
[stat()]:           https://pubs.opengroup.org/onlinepubs/9699919799/functions/stat.html
[strsignal()]:      https://pubs.opengroup.org/onlinepubs/9699919799/functions/strsignal.html
[sysconf()]:        https://pubs.opengroup.org/onlinepubs/9699919799/functions/sysconf.html
[truncate()]:       https://pubs.opengroup.org/onlinepubs/9699919799/functions/truncate.html
[waitpid()]:        https://pubs.opengroup.org/onlinepubs/9699919799/functions/waitpid.html
[write()]:          https://pubs.opengroup.org/onlinepubs/9699919799/functions/write.html

[execvpe]:          https://man7.org/linux/man-pages/man3/execvpe.3.html
[flock-lin]:        https://man7.org/linux/man-pages/man2/flock.2.html
[mkostemps-lin]:    https://man7.org/linux/man-pages/man3/mkstemp.3.html
[setgroups-lin]:    https://man7.org/linux/man-pages/man2/getgroups.2.html
[sigabbrev_np()]:   https://man7.org/linux/man-pages/man3/sigabbrev_np.3.html

[flock-mac]:        https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/flock.2.html
[lchmod-mac]:       https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man3/lchmod.3.html
[mkostemps-mac]:    https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man3/mkstemp.3.html
[setgroups-mac]:    https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/setgroups.2.html

[flock-bsd]:        https://man.freebsd.org/cgi/man.cgi?query=flock
[lchmod-bsd]:       https://man.freebsd.org/cgi/man.cgi?query=lchmod
[mkostemps-bsd]:    https://man.freebsd.org/cgi/man.cgi?query=mkostemp
[sys_signame]:      https://man.freebsd.org/cgi/man.cgi?query=sys_signame
[posix_spawn_file_actions_addclosefrom_np]: https://man.freebsd.org/cgi/man.cgi?query=posix_spawn_file_actions_addclosefrom_np
[posix_spawn_file_actions_addchdir_np]: https://man.freebsd.org/cgi/man.cgi?query=posix_spawn_file_actions_addchdir_np
[setgroups-bsd]:    https://man.freebsd.org/cgi/man.cgi?query=setgroups

[flock-ill]:        https://illumos.org/man/3C/flock
[mkostemps-ill]:    https://illumos.org/man/3C/mkostemp
[setgroups-ill]:    https://illumos.org/man/2/setgroups

<!-- Links -->


| Name           | Exposed by                   | Header       | Availability
|----------------|------------------------------|--------------|--------------
|[bind()]        | `bindSocket()`               | [socket.h]   | 
|[chdir()]       | `changeDirectory()`          | [file.h]     | 
|[chmod()]       | `changeMode()`               | [file.h]     | 
|[chown()]       | `changeOwner()`              | [file.h]     | 
|[chroot()]      | `changeRoot()`               | [file.h]     | Removed from Posix but universally available
|[close()]       | `FileDescriptor::~FileDescriptor()`, `FileDescriptor::close()` | [file.h] | 
|[dup()]         | `duplicate()`                | [file.h]     | 
|[dup2()]        | `duplicateTo()`              | [file.h]     | 
|[exec()] family | `exec()`, `execp()`          | [spawn.h]    | An overload of `execp()` that takes environment is only available on platforms that support `execvpe()` call: [Linux][execvpe], OpenBSD.
|[fchdir()]      | `changeDirectory()`          | [file.h]     | 
|[fchmod()]      | `changeMode()`               | [file.h]     | 
|[fchown()]      | `changeOwner()`              | [file.h]     | 
|`flock()`       | `lockFile()`, `tryLockFile()`, `unlockFile()` | [file.h] | [Linux][flock-lin], [Mac][flock-mac], [BSD][flock-bsd], [Illumos][flock-ill]
|[fork()]        | `forkProcess()`              | [spawn.h]    |
|[fstat()]       | `getStatus()`                | [file.h]     |
|[ftruncate()]   | `truncateFile()`             | [file.h]     |
|[getgrnam_r()]  | `Group::getByName()`         | [users.h]    |
|[getgroups()]   | `getGroups()`                | [identity.h] |
|[getgruid_r()]  | `Group::getById()`           | [users.h]    |
|[gethostname()] | `getHostName()`              | [system.h]   |
|[getpwnam_r()]  | `Passwd::getByName()`        | [users.h]    |
|[getpwuid_r()]  | `Passwd::getById()`          | [users.h]    |
|[getsockname()] | `getSocketName()`            | [socket.h]   |
|[getsockopt()]  | `getSocketOption()`          | [socket.h]   |
|[kill()]        | `sendSignal()`               | [signal.h]   | 
|`lchmod()`      | `changeLinkMode()`           | [file.h]     | [Mac][lchmod-mac], [BSD][lchmod-bsd]
|[lchown()]      | `changeLinkOwner()`          | [file.h]     | 
|[lstat()]       | `getLinkStatus()`            | [file.h]     | 
|`mkostemps()`   | `FileDescriptor::openTemp()` | [file.h]     | [Linux][mkostemps-lin], [Mac][mkostemps-mac], [BSD][mkostemps-bsd], [Illumos][mkostemps-ill]
|[mkdir()]       | `makeDirectory()`            | [file.h]     | 
|[mkdirat()]     | `makeDirectoryAt()`          | [file.h]     | 
|[mmap()]        | `MemoryMap`                  | [file.h]     | 
|[munmap()]      | `MemoryMap`                  | [file.h]     | 
|[open()]        | `FileDescriptor::open()`     | [file.h]     | 
|[pipe()]        | `Pipe::create()`             | [file.h]     | 
|`posix_spawn_file_actions_addchdir_np()`     | `SpawnFileActions::addChdirNp()`     | [spawn.h] | Mac (see local man page), [BSD][posix_spawn_file_actions_addchdir_np]
|[posix_spawn_file_actions_addclose()]        | `SpawnFileActions::addClose()`       | [spawn.h] |
|`posix_spawn_file_actions_addclosefrom_np()` | `SpawnFileActions::addCloseFromNp`   | [spawn.h] | [BSD][posix_spawn_file_actions_addclosefrom_np]
|[posix_spawn_file_actions_adddup2()]         | `SpawnFileActions::addDuplicateTo()` | [spawn.h] |
|`posix_spawn_file_actions_addinherit_np()`   | `SpawnFileActions::addInheritNp()`   | [spawn.h] | Mac (see local man page)
|[posix_spawn_file_actions_addopen()]         | `SpawnFileActions::addOpen()`        | [spawn.h] |
|[posix_spawn_file_actions_init()], [posix_spawn_file_actions_destroy()] | `SpawnFileActions` class | [spawn.h] |
|[posix_spawnattr_init()], [posix_spawnattr_destroy()] | `SpawnAttr` class | [spawn.h] |
|[posix_spawnattr_setflags()]                   | `SpawnAttr::setFlags()`            | [spawn.h] |
|[posix_spawnattr_setsigdefault()]              | `SpawnAttr::setSigDefault()`       | [spawn.h] |
|[posix_spawnattr_setpgroup()]                  | `SpawnAttr::setPGroup()`           | [spawn.h] |
|[posix_spawn()] | `spawn()`                    | [spawn.h]    | Mapped to `_spawn()` on Win32
|[posix_spawnp()]| `spawn()`                    | [spawn.h]    | Mapped to `_spawnp()` on Win32
|[raise()]       | `raiseSignal()`              | [signal.h]   | 
|[read()]        | `readFile()`                 | [file.h]     | 
|[recv()]        | `receiveSocket()`            | [socket.h]   |
|[recvfrom()]    | `receiveSocket()`            | [socket.h]   | 
|[recvmsg()]     | `receiveSocket()`            | [socket.h]   | 
|[send()]        | `sendSocket()`               | [socket.h]   |
|[sendto()]      | `sendSocket()`               | [socket.h]   |
|[sendmsg()]     | `sendSocket()`               | [socket.h]   |
|[setgid()]      | `setGid()`                   | [identity.h] |
|[setegid()]     | `setEffectiveGid()`          | [identity.h] |
|[seteuid()]     | `setEffectiveUid()`          | [identity.h] |
|`setgroups()`   | `setGroups()`                | [identity.h] | [Linux][setgroups-lin], [Mac][setgroups-mac], [BSD][setgroups-bsd], [Illumos][setgroups-ill]
|[setpgid()]     | `setProcessGroupId()`        | [process.h]  |
|[setsid()]      | `setSessionId()`             | [process.h]  |
|[setsockopt()]  | `setSocketOption()`          | [socket.h]   |
|[setuid()]      | `setUid()`                   | [identity.h] |
|[sigabbrev_np()], [sys_signame], `sys_sigabbrev` | `signalName()` | [signal.h] | Various non-portable ways of obtaining a signal name. The `signalName()` is supported on all platforms and falls back on returning signal number converted to string if no known mapping is available
|[sigaction()]   | `setSignalAction()`, `getSignalAction()` | [signal.h] | Note that `struct sigaction` is minimally wrapped by `SignalAction` class
|[sigaddset()]   | `SignalSet::add()`           | [signal.h]   |
|[sigdelset()]   | `SignalSet::del()`           | [signal.h]   |
|[sigemptyset()] | `SignalSet::SignalSet()`, `SignalSet::none()` | [signal.h] |
|[sigfillset()]  | `SignalSet::all()`           | [signal.h]   |
|[sigismember()] | `SignalSet::isMember()`      | [signal.h]   |
|[signal()]      | `setSignalHandler()`         | [signal.h]   |
|[sigprocmask()] | `setSignalProcessMask()`, `getSignalProcessMask()`| [signal.h] |
|[socket()]      | `createSocket()`             | [socket.h]   |
|[stat()]        | `getStatus()`                | [file.h]     | 
|[strsignal()]   | `signalMessage()`            | [signal.h]   | 
|[sysconf()]     | `systemConfig()`             | [system.h]   |
|[truncate()]    | `truncateFile()`             | [file.h]     |
|[waitpid()]     | `ChildProcess::~ChildProcess()`, `ChildProcess::wait()` | [process.h] | 
|[write()]       | `writeFile()`                | [file.h]     | 
