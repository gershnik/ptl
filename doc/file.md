# File Operations

<!--
 Notes to AI grammar checkers:
   - this document uses Posix in preference to POSIX.
   - this document does not require pedantic comma after e.g.
-->

<!-- TOC depthfrom:2 -->

- [Overview](#overview)
- [FileDescriptor](#filedescriptor)
    - [Lifecycle](#lifecycle)
    - [Opening files](#opening-files)
    - [Temporary files](#temporary-files)
- [File-like arguments](#file-like-arguments)
- [Duplicating file descriptors](#duplicating-file-descriptors)
- [Reading and writing files](#reading-and-writing-files)
- [Advisory file locking](#advisory-file-locking)
- [File owner, mode and status](#file-owner-mode-and-status)
- [Truncating files](#truncating-files)
- [Pipes](#pipes)
- [Memory maps](#memory-maps)
- [Directory operations](#directory-operations)
- [Notes on Windows](#notes-on-windows)

<!-- /TOC -->

## Overview

The `<ptl/file.h>` header provides facilities for working with files, file descriptors, pipes, memory maps, and directories. It contains:

- An RAII wrapper, `FileDescriptor`, that owns a file descriptor and closes it when it goes out of scope.
- A `FileDescriptorLike` concept and a `c_fd` free function that let PTL methods accept raw `int` descriptors, `FILE *`, `FileDescriptor` objects, and your own file-like types.
- Free functions wrapping the common file-related Posix calls (`read`, `write`, `chmod`, `chown`, `flock`, `stat`, `truncate`, `mkdir`, `chdir` and more), each accepting either a path or a file descriptor where applicable.
- The `Pipe` struct, the `MemoryMap` RAII wrapper, and the `FileLock` enumeration.

Everything is under `namespace ptl`. As with the rest of PTL, methods come in two forms: one that throws on failure and one that takes a trailing error code argument. See the [Error Handling](usage.md#error-handling) section of the usage guide for the general rules.

## FileDescriptor

### Lifecycle

`FileDescriptor` is the RAII wrapper. It owns at most one open file descriptor.

```cpp
#include <ptl/file.h>
using namespace ptl;

FileDescriptor empty;             //owns nothing
FileDescriptor fd(rawFd);         //takes ownership of an existing int descriptor
```

The default constructor produces an empty object. The `int` constructor is explicit and takes ownership of the supplied descriptor. The destructor closes the descriptor if there is one.

The object is move-only. Moving leaves the source empty. Assignment is by value, so the previous descriptor (if any) is closed when the assignment completes.

```cpp
FileDescriptor a = FileDescriptor::open("file_a", O_RDONLY);
FileDescriptor b = std::move(a);    //a is now empty, b owns the descriptor
b = FileDescriptor::open("file_b", O_RDONLY);  //the previous fd held by b is closed
```

Conversion to `bool` is true when the object owns a descriptor.

```cpp
if (fd) {
    //fd owns a descriptor
}
```

You can extract the raw descriptor without giving up ownership using `get`, or release ownership with `detach`:

```cpp
int raw = fd.get();       //fd still owns the descriptor
int raw2 = fd.detach();   //fd is now empty, caller is responsible for closing
```

To explicitly close before the object goes out of scope, call `close`:

```cpp
fd.close();   //equivalent to fd = FileDescriptor()
```

### Opening files

The static `FileDescriptor::open` method wraps the Posix `open` call. It accepts the same flags and mode arguments and returns a new `FileDescriptor`.

```cpp
try {
    auto fd = FileDescriptor::open("test_file",
                                   O_WRONLY | O_CREAT,
                                   S_IRWXU | S_IRWXG | S_IRWXO);
    //use fd
} catch (std::system_error & ex) {
    //handle failure
}
```

The same call using the error code form looks like this:

```cpp
std::error_code ec;
auto fd = FileDescriptor::open("test_file", O_WRONLY | O_CREAT,
                               S_IRWXU | S_IRWXG | S_IRWXO, ec);
if (ec) {
    //handle failure, fd is empty
}
```

When the file does not need to be created, you can omit the mode argument:

```cpp
auto fd = FileDescriptor::open("test_file", O_RDONLY);
```

The path can be any type that satisfies the `PathLike` concept: a string literal, `std::string`, `std::filesystem::path`, and so on. You can make your own string-like types acceptable by specializing the `CPathTraits` template.

```cpp
std::string name = "some_file";
auto fd1 = FileDescriptor::open(name, O_RDONLY);

std::filesystem::path path = "some_file";
auto fd2 = FileDescriptor::open(path, O_RDONLY);
```

Sometimes you want to treat an expected error as a non-error. The classic case is "open this if it exists, otherwise it is fine that it does not". The `AllowedErrors` helper, defined in `<ptl/errors.h>`, lets you whitelist specific error codes:

```cpp
#include <ptl/errors.h>

try {
    AllowedErrors<ENOENT> ec;
    auto fd = FileDescriptor::open("maybe_nonexistent", O_RDONLY, ec);
    if (ec) {
        //ENOENT happened, other errors would have thrown
    }
} catch (std::system_error & ex) {
    //something unexpected
}
```

### Temporary files

On platforms that provide `mkostemps`, the static `FileDescriptor::openTemp` method creates a uniquely named temporary file and returns a `FileDescriptor` open on it.

```cpp
char nameTemplate[] = "/tmp/myappXXXXXX";
auto fd = FileDescriptor::openTemp(nameTemplate);
//nameTemplate has been modified in place to the actual filename
//for example "/tmp/myappa1b2c3"
```

The template string is modified in place, so it must be a writable buffer (not a string literal). The trailing `XXXXXX` is replaced with a unique suffix.

If you need the generated name to end with a fixed suffix, pass the suffix length as a second argument:

```cpp
char nameTemplate[] = "/tmp/myappXXXXXX.txt";
auto fd = FileDescriptor::openTemp(nameTemplate, 4);   //4 = length of ".txt"
```

You can also pass extra `open` flags (such as `O_CLOEXEC`) as a third argument:

```cpp
auto fd = FileDescriptor::openTemp(nameTemplate, 4, O_CLOEXEC);
```

This method is only declared when PTL detects `mkostemps` at configuration time. If you need a portable temporary file across all platforms, fall back to constructing the path yourself and using `FileDescriptor::open` with `O_CREAT | O_EXCL`.

## File-like arguments

PTL methods that take a file descriptor do not require a `FileDescriptor`. They accept anything that satisfies the `FileDescriptorLike` concept, which means anything whose descriptor can be extracted via `FileDescriptorTraits`. Out of the box this covers `int`, `FileDescriptor` and `FILE *`:

```cpp
FileDescriptor fd = FileDescriptor::open("a", O_RDONLY);
int raw = ::open("b", O_RDONLY);
FILE * fp = fopen("c", "r");

changeMode(fd, S_IRUSR | S_IWUSR);    //FileDescriptor
changeMode(raw, S_IRUSR | S_IWUSR);   //raw int
changeMode(fp, S_IRUSR | S_IWUSR);    //FILE * (fileno is called internally)
```

This means you can mix PTL with code that uses plain Posix calls, or with code that uses C stdio, without conversion friction.

You can also make your own type work as a file by specializing `FileDescriptorTraits`:

```cpp
class MyFile { ... };

template<> struct ptl::FileDescriptorTraits<MyFile> {
    static int c_fd(const MyFile & f) noexcept { return f.descriptor(); }
};

//now MyFile can be passed wherever FileDescriptorLike is accepted
```

If you only want the raw integer descriptor for some reason, call `c_fd`:

```cpp
int n = c_fd(fd);   //same as fd.get() for FileDescriptor
int n2 = c_fd(5);   //identity for int
```

## Duplicating file descriptors

You can duplicate file-like objects' descriptors using `duplicate` and `duplicateTo`. These wrap the `dup` and `dup2` Posix calls.

```cpp
auto fd1 = FileDescriptor::open("some_file", O_RDONLY);

//create a new descriptor referring to the same file
auto fd2 = duplicate(fd1);

//duplicate into an existing slot, closing whatever was there
duplicateTo(fd1, stdout);
```

These functions always throw on failure and do not offer the error code form. The possible failures of `dup` and `dup2` are all of the "bug in your logic" variety (bad source descriptor, too many open files), so PTL treats them as exceptions only.

## Reading and writing files

`readFile` and `writeFile` wrap `read` and `write`. They accept any file-like object.

```cpp
FileDescriptor fd = ...;

char buf[20];
auto readCount = readFile(fd, buf, sizeof(buf));
auto writeCount = writeFile(fd, buf, sizeof(buf));
```

As usual, you can add an error code argument:

```cpp
std::error_code ec;
auto readCount = readFile(fd, buf, sizeof(buf), ec);
if (ec) {
    //handle failure
}
```

The byte count is of type `io_size_t` and the return is `io_ssize_t`. On most platforms these are `size_t` and `ssize_t`. On Windows the underlying CRT call takes a narrower count type, so PTL checks for overflow at the wrapper boundary. Requesting a size that the underlying call cannot represent unconditionally throws `std::system_error` with `EINVAL`, since this kind of overflow is a logic bug rather than a runtime condition.

## Advisory file locking

The `flock` family of Posix calls is wrapped by `lockFile`, `tryLockFile` and `unlockFile`. The semantics and names are deliberately shaped to make it easy to implement a [_Lockable_](https://en.cppreference.com/w/cpp/named_req/Lockable.html) on top of them.

These use the following enumeration:

```cpp
enum class FileLock : int {
    Shared = LOCK_SH,
    Exclusive = LOCK_EX
};
```

`lockFile` blocks until the lock is acquired. `tryLockFile` returns `true` if the lock was acquired and `false` if it would have blocked (it does not throw on `EWOULDBLOCK`, it returns `false` instead). `unlockFile` releases the lock.

Here is a small example that implements a file-based mutex satisfying the standard library's `Lockable` requirements:

```cpp
template<ptl::FileDescriptorLike T>
class FileMutex {
public:
    FileMutex(T & file) noexcept : m_file(file) {}

    void lock()      { lockFile(m_file, FileLock::Exclusive); }
    bool try_lock()  { return tryLockFile(m_file, FileLock::Exclusive); }

    void unlock() noexcept {
        std::error_code ec;
        unlockFile(m_file, ec);
        if (ec)
            std::terminate();
    }
private:
    T & m_file;
};

FileDescriptor someFile = ...;

void foo() {
    const std::lock_guard lock(someFile);
    //critical section
}
```

These functions are Posix only.

## File owner, mode and status

PTL provides the expected functions to set and query file ownership and mode, and to query file status. These methods operate on either file-like objects or paths, and have variants for symbolic links.

Arguably, the path-based forms are not strictly necessary since `std::filesystem` provides equivalent functionality portably. However, portability has a downside: some OS-specific extensions are not exposed via `std::filesystem`, whereas PTL methods do expose them.

As usual, all these functions support both exception and error code modes.

```cpp
FileDescriptor fd = ...;

//wrappers for fchown, chown and lchown
changeOwner(fd, /*uid*/0, /*gid*/0);
changeOwner("some_file", /*uid*/0, /*gid*/0);
changeLinkOwner("some_symlink", /*uid*/0, /*gid*/0);

//wrappers for fchmod, chmod and lchmod
changeMode(fd, S_IRUSR | S_IWUSR);
changeMode("some_file", S_IRUSR | S_IWUSR);
changeLinkMode("some_symlink", S_IRUSR | S_IWUSR);

//wrappers for fstat, stat and lstat
struct ::stat st;
getStatus(fd, st);
getStatus("some_file", st);
getLinkStatus("some_symlink", st);
```

These functions are Posix only. `changeLinkMode` additionally requires the `lchmod` system call, which is detected at configuration time.

## Truncating files

You can truncate files via file-like objects or paths. The `truncateFile` call wraps `ftruncate` and `truncate`.

```cpp
FileDescriptor fd = ...;

truncateFile(fd, 5);
truncateFile("some_file", 5);
```

This function is Posix only.

## Pipes

Pipes are represented by the `Pipe` struct with two `FileDescriptor` members: `readEnd` and `writeEnd`.

```cpp
#include <ptl/file.h>
using namespace ptl;

Pipe p = Pipe::create();
auto rcount = readFile(p.readEnd, buf, size);
//...
auto wcount = writeFile(p.writeEnd, buf, size);
```

Since `Pipe` is a struct, you can use structured bindings to get the ends directly:

```cpp
auto [readEnd, writeEnd] = Pipe::create();
```

`Pipe::create` is available on Windows; the call is implemented via `_pipe` with `_O_BINARY`.

## Memory maps

Memory maps (the thing you get from an `mmap` call) are represented by the `MemoryMap` RAII wrapper.

```cpp
#include <ptl/file.h>
using namespace ptl;

try {
    auto fd = FileDescriptor::open("some_file", O_RDONLY);
    struct ::stat st;
    getStatus(fd, st);
    MemoryMap map(nullptr, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    auto fileContent = std::string_view(static_cast<char *>(map.data()), map.size());
} catch (std::system_error & ex) {
    //handle failure
}
```

The arguments to the `MemoryMap` constructor are the same as those of the `mmap` call. As usual, you can pass any file-like object, not just a `FileDescriptor`. You can also omit the usually defaulted initial pointer and offset arguments:

```cpp
MemoryMap map(st.st_size, PROT_READ, MAP_PRIVATE, fd);
```

You can also pass an error code object as the last argument:

```cpp
std::error_code ec;
MemoryMap map(st.st_size, PROT_READ, MAP_PRIVATE, fd, ec);
if (ec) {
    //handle failure
}
```

The `MemoryMap` object is convertible to `bool`. It is false for an invalid mapping, which makes the following idiom convenient:

```cpp
std::error_code ec;
if (MemoryMap map(st.st_size, PROT_READ, MAP_PRIVATE, fd, ec)) {
    //success
} else {
    //handle ec
}
```

`MemoryMap` is move-only. It has a default constructor that creates an invalid mapping and a `close` method that releases the mapping early. The `data` and `size` methods return the mapping start (as `void *`) and its size in bytes.

This class is Posix only. Memory mapping on Windows uses a different API and is not wrapped here.

## Directory operations

PTL provides wrappers for the common directory-related operations.

As usual, all these functions can operate in either exception or error code mode.

```cpp
#include <ptl/file.h>
using namespace ptl;

//wrappers for mkdir/mkdirat
makeDirectory("somedir", S_IRUSR | S_IWUSR | S_IXUSR);
FileDescriptor fdparent = ...;
makeDirectoryAt(fdparent, "somedir", S_IRUSR | S_IWUSR | S_IXUSR);

//wrappers for chdir/fchdir
changeDirectory("somedir");
FileDescriptor fddir = ...;
changeDirectory(fddir);

//wrapper for chroot
changeRoot("somedir");
```

These functions are Posix only.

## Notes on Windows

Most of `<ptl/file.h>` works on Windows, but the surface is narrower than on Posix.

The Windows-supported facilities are:

- `FileDescriptor` and its lifecycle methods, including `FileDescriptor::open`.
- `readFile` and `writeFile`.
- `duplicate` and `duplicateTo`.
- `Pipe::create`.

The following facilities are Posix only and are not declared on Windows:

- `lockFile`, `tryLockFile`, `unlockFile` and the `FileLock` enumeration.
- `changeOwner`, `changeLinkOwner`, `changeMode`, `changeLinkMode`.
- `getStatus`, `getLinkStatus`.
- `truncateFile`.
- `makeDirectory`, `makeDirectoryAt`, `changeDirectory`, `changeRoot`.
- `MemoryMap`.
- `FileDescriptor::openTemp`.

For functionality not covered here, fall back to `std::filesystem` (which is portable) or to the Windows API directly.
