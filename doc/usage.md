# PTL usage guide

<!-- TOC depthfrom:2 -->

- [Basics](#basics)
- [Naming](#naming)
- [Error Handling](#error-handling)
- [Examples](#examples)
    - [FileDescriptor objects and file operations](#filedescriptor-objects-and-file-operations)
    - [Pipes](#pipes)

<!-- /TOC -->

## Basics

To access PTL you will need to either include individual `ptl/foo.h` headers (see [Function Mapping](doc/function-mapping.md) for which header provides what functionality) or simply include an umbrella header `ptl/ptl.h`.

Everything in the library is under `namespace ptl`. 

## Naming

PTL uses `CamelCase` for types and `lowerUpper` case for methods. It also mostly avoids method names that would be identical to raw Posix calls. This is done for a reason. Two reasons actually.
1. Some Posix calls can actually be implemented as macros (on _some platforms_ to make it worse). You can imagine the resulting havoc if a `foo` method name in PTL clashed with a macro. While there are ways around it that can be used in the library, the problem usually leaks to library users too.
2. Even without macros you should be able to say `using namespace ptl` and then call its methods without `ptl::` prefix and not have to deal with "ambiguous call" errors when the name and argument types match the raw Posix calls. Or worse having the raw method being called when you thought you called a safe wrapper. Using a different naming convention and making names more elaborate (e.g. `duplicate` for `dup` or `makeDirectory` for `mkdir`) avoids this issue. 


## Error Handling

PTL uses the strategy similar (but not identical!) to one popularized by `std::filesystem` (and `boost::filesystem` before).

Most methods that can fail can be called in two ways: `foo(<arguments>)` or `foo(<arguments>, ec)`. The first form throws an exception of failure. By default, it is `std::exception` but this can be globally overridden with another exception type or replaced with program termination. The second form returns the error in output parameter `ec` or clears it on success. This parameter type is by default `std::error_code` but you make any call accept `boost::error_code` or your own class by specializing some traits. Unlike typical implementation of `std::filesystem` this is done without code duplication via some template magic. 

Some calls - those that can only legitimately fail if there is a logic error in your code (e.g. `EINVAL`) or if "the world is falling apart" always throw exceptions and do not allow error code form. The thinking is that even in exception-free code the only reasonable response to such errors is to terminate the application so there is no point in increasing client complexity here.

PTL does not use, nor plans to use `std::expected` or `outcome`. This is partly because the first is not commonly available yet and the second will bring another library dependency. More importantly `outcome` style of programming unconditionally penalizes people who want to use exceptions - they pay for what they don't use both in terms of mental burden (don't forget to dereference that outcome) and also in generated code (whether the compiler could always see through `outcome` manipulations and inline them out is unclear).

## Examples

The following sections demonstrate main PTL usage concepts. This is not an extensive reference of all available functionality. See [Function Mapping](doc/function-mapping.md) for full list of available APIs.

### FileDescriptor objects and file operations

`FileDescriptor` is a RAII wrapper over Posix file descriptor. 

PTL methods that work on `FileDescriptor` objects also usually work on plain C `int` file descriptors as well as on 
`FILE *` C wrappers.

The following example uses exceptions as its error handling strategy.

```cpp
#include <ptl/file.h>
using namespace ptl;

try {
    auto fd = FileDescriptor::open("test_file", 
                                   O_WRONLY | O_CREAT, 
                                   S_IRWXU | S_IRWXG | S_IRWXO);
    
    changeMode(fd, S_IRUSR | S_IWUSR);
    changeOwner(fd, uid, gid); 

    int plain_fd = open(...);
    changeMode(plain_fd, S_IRUSR | S_IWUSR);
    changeOwner(plain_fd, uid, gid); 

    FILE * fp = fopen(...);
    changeMode(fp, S_IRUSR | S_IWUSR);
    changeOwner(fp, uid, gid); 

} catch (std::system_error & ex) {
    ...
}

```

Here is the same but using `std::error_code` (you can use other types of error codes too).

```cpp
#include <ptl/file.h>
using namespace ptl;

bool foo(std::error_code & ec) {
    auto fd = FileDescriptor::open("test_file", 
                                   O_WRONLY | O_CREAT, 
                                   S_IRWXU | S_IRWXG | S_IRWXO,
                                   ec);
    if (ec)
        return false;

    changeMode(fd, S_IRUSR | S_IWUSR, ec);
    if (ec)
        return false;
    
    changeOwner(fd, uid, gid, ec); 
    if (ec)
        return false;

    //etc.
}
```

When using exceptions sometimes you want to **not** have an exception be thrown on certain errors because they are _expected_. The classical example is opening a file. Sometimes the file _must_ be there and sometimes the file not existing is completely fine. PTL lets you handle this cleanly:

```cpp
try {
    AllowedErrors<ENOENT, EDOM> ec;
    auto fd = FileDescriptor::open("maybe_nonexistant", O_RDONLY, ec);
    if (ec) {
        //either ENOENT or EDOM
        //other errors will throw
        //handle file not present
    }
} catch (std::system_error & ex) {
    //something bad and unexpected happened
}
```

In the examples above the filename was given as a simple string literal. Any PTL method that takes a path actually accepts a variety of types: `std::string_view`, `std::filesystem::path`, `std::string` and more. (You can even make your own types acceptable by specializing some traits)

```cpp
std::string filename_str = "some_file";
auto fd1 = FileDescriptor::open(filename_str, O_RDONLY);

std::filesystem::path filename_path = "some_file";
auto fd2 = FileDescriptor::open(filename_path, O_RDONLY);

std::string_view dir_str = "/some/dir";
makeDirectory(dir_str, S_IRWXU | S_IRWXG);

std::filesystem::path dir_path = "/another/dir";
makeDirectory(dir_path, S_IRWXU | S_IRWXG);

```

### Pipes

Pipes are represented by `Pipe` struct with two `FileDescriptor` members: `readEnd` and `writeEnd`.

```cpp
#include <ptl/file.h>
using namespace ptl;

Pipe p = Pipe::create();
auto rcount = readFile(p.readEnd, buf, size);
...
auto wcount = writeFile(p.writeEnd, buf, size);
```

Since `Pipe` is a struct you can use structural bindings to get the ends directly:

```cpp
auto [readEnd, writeEnd] = Pipe::create();
```



