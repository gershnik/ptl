# PTL

[![Language](https://img.shields.io/badge/language-C++-blue.svg)](https://isocpp.org/)
[![Standard](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization)
[![License](https://img.shields.io/badge/license-BSD-brightgreen.svg)](https://opensource.org/licenses/BSD-3-Clause)


A C++ library for Posix and related calls.

<!-- TOC depthfrom:2 -->

- [Purpose](#purpose)
- [Features](#features)
- [Naming](#naming)
- [Extensions](#extensions)
- [Android compatibility](#android-compatibility)
- [Emscripten/Wasm compatibility](#emscriptenwasm-compatibility)
- [Win32/MinGW compatibility](#win32mingw-compatibility)
- [Error Handling](#error-handling)
- [Integration](#integration)
    - [CMake via FetchContent](#cmake-via-fetchcontent)
    - [Building and installing on your system](#building-and-installing-on-your-system)
        - [Basic use](#basic-use)
        - [CMake package](#cmake-package)
        - [Via pkg-config](#via-pkg-config)
    - [Copying to your sources](#copying-to-your-sources)
- [Configuration](#configuration)
- [Usage](#usage)

<!-- /TOC -->

## Purpose
Every time a C++ developer needs to issue some standard system call on Linux/Mac/{Free|Open|Net}BSD/Illumos he has to deal with the same old, same old annoying chores. You need to figure out its error reporting strategy (very often there are some peculiarities and gotchas to specific calls!) and adapt it to C++. You need to convert the arguments from C++ to the form required by the call and do the reverse for output. Sometimes it is as easy as `.c_str()`, sometimes much more convoluted. If there is a do/undo semantics one needs to create a RAII class yet again and figure out all the little details that go with it. 

There is, unfortunately, nothing in C++ like Python's `os` module. The standard library steadfastly refuses to provide OS-specific calls and want everything to be fully portable. 
This is extremely annoying, and, since I couldn't find an existing library that addressed this problem to my satisfaction, I've decided to create this one.

PTL doesn't strive to be a comprehensive Posix wrapper (at least not initially). It contains the calls I personally needed at one point or another. I intend to extend it as need arises.
See [Function Mapping](doc/function-mapping.md) for currently supported functions and which part of PTL exposes them.

## Features
* Header only
* Depends only on C++20 standard library. If and only if your standard library is missing `std::format` (which is all too common in 2023) you will need to have `fmt` available instead.
* Thin. The functions and classes it provides are minimal inline safe wrappers of Posix functionality. There is no attempt to provide a higher level of abstraction unless it is absolutely necessary for safety. Similarly, there are no helpful utilities (like for example "read all content of file" or "spawn a process and read its output into a string") which are trivial to implement on top of wrapped calls. These are nice but the belong to a different library.
* Supports both error code returns (`std::error_code`, `boost::error_code` or any error code class you might want to use!) and exceptions _without code duplication_. See [Error Handling](#error-handling) below for details.
* RAII everywhere there is do/undo semantics.
* Free functions that take conceptual arguments rather than class methods whenever practical. For example, using this library you can say `duplicate(STDOUT_FILENO)` or `duplicate(stdout)` or `duplicate(an_instance_of_file_descriptor_class)` and they all will do the same thing: duplicate the file descriptor and produce a new instance of file descriptor class. The `duplicate` function takes anything that matches a "file like" concept. This way you can mix and match PTL with plain Posix code as desired. It is not all or nothing proposition. You can even make PTL work with your own classes if you have them by matching them to a concept (via traits specialization usually).

## Naming

PTL uses `CamelCase` for types and `lowerUpper` case for methods. It also mostly avoids method names that would be identical to raw Posix calls. This is done for a reason. Two reasons actually.
1. Some Posix calls can actually be implemented as macros (on _some platforms_ to make it worse). You can imagine the resulting havoc if a `foo` method name in PTL clashed with a macro. While there are ways around it that can be used in the library, the problem usually leaks to library users too.
2. Even without macros you should be able to say `using namespace ptl` and then call its methods without `ptl::` prefix and not have to deal with "ambiguous call" errors when the name and argument types match the raw Posix calls. Or worse having the raw method being called when you thought you called a safe wrapper. Using a different naming convention and making names more elaborate (e.g. `duplicate` for `dup` or `makeDirectory` for `mkdir`) avoids this issue. 

## Extensions

In addition to portable Posix functionality PTL also includes support for some common extensions. If you use PTL via CMake or build and install it the extensions supported on your platform will be automatically detected and enabled. If you simply grab PTL headers then you will need to manually set various enabling macros to 1 in order to enable them. See [Configuration](#configuration) for more details.

## Android compatibility

PTL can be used on Android. However, note that some functionality may not be available based on what Posix calls are provided on given Android API level. Generally, all things should work above API level 30. Also note that Android often deviates from exact Posix semantics of many calls even if the calls themselves are available. In some cases, where the deviation can affect safety (e.g. `gethostname` not null terminating truncated output)
or is especially annoying (`getpwnam_r` producing hard error when username is not found) PTL 'fixes' the issue. In other cases (like `posix_spawn` not reporting error when executable is not found) you will need to deal with differences in your own code. 

## Emscripten/Wasm compatibility

PTL can be used on Wasm (at least with Emscripten, possibly with other toolchains too). Note that Emscripten often "provides" many Posix calls
it doesn't implement in its headers and makes them either not link or fail at compile time. PTL cannot detect this during configuration and thus
you will need to deal with such cases in your own code. 

## Win32/MinGW compatibility

PTL can be used on Win32/MinGW but it only exposes very limited Posix-compatibility functionality provided be these environments. If you need a full portability between Unix and Windows you should look for a portable library of some sort. 
Note that PTL is fully supported on Cygwin as it is a "normal" Posix-like environment.

## Error Handling

PTL uses the strategy similar (but not identical!) to one popularized by `std::filesystem` (and `boost::filesystem` before).

Most methods that can fail can be called in two ways: `foo(<arguments>)` or `foo(<arguments>, ec)`. The first form throws an exception of failure. By default, it is `std::exception` but this can be globally overridden with another exception type or replaced with program termination. The second form returns the error in output parameter `ec` or clears it on success. This parameter type is by default `std::error_code` but you make any call accept `boost::error_code` or your own class by specializing some traits. Unlike typical implementation of `std::filesystem` this is done without code duplication via some template magic. 

Some calls - those that can only legitimately fail if there is a logic error in your code (e.g. `EINVAL`) or if "the world is falling apart" always throw exceptions and do not allow error code form. The thinking is that even in exception-free code the only reasonable response to such errors is to terminate application so there is no point in increasing client complexity here.

PTL does not use, nor plans to use `std::expected` or `outcome`. This is partly because the first is not commonly available yet and the second will bring another library dependency. More importantly `outcome` style of programming unconditionally penalizes people who want to use exceptions - they pay for what they don't use both in terms of mental burden (don't forget to dereference that outcome) and also in generated code (whether the compiler could always see through `outcome` manipulations and inline them out is unclear).

## Integration

### CMake via FetchContent

```cmake
include(FetchContent)
...
FetchContent_Declare(ptl
    GIT_REPOSITORY  https://github.com/gershnik/ptl.git
    GIT_TAG         v1.0  #use the tag, branch or sha you need
    GIT_SHALLOW     TRUE
)
...
FetchContent_MakeAvailable(ptl)
...
target_link_libraries(mytarget
PRIVATE
  ptl::ptl
)
```
> ℹ&#xFE0F; _[What is FetchContent?](https://cmake.org/cmake/help/latest/module/FetchContent.html)_

### Building and installing on your system

You can also build and install PTL on your system using CMake.

1. Download or clone this repository into SOME_PATH
2. On command line:
```bash
cd SOME_PATH
cmake -S . -B build 
cmake --build build

#Optional
#cmake --build build --target run-tests

#install to /usr/local
sudo cmake --install build
#or for a different prefix
#cmake --install build --prefix /usr
```

Once PTL has been installed it can be used int the following ways:

#### Basic use 

Set the include directory to `<prefix>/include` where `<prefix>` is the install prefix from above.

#### CMake package

```cmake
find_package(ptl)

target_link_libraries(mytarget
PRIVATE
  ptl::ptl
)
```

#### Via `pkg-config`

Add the output of `pkg-config --cflags ptl` to your compiler flags.

Note that the default installation prefix `/usr/local` might not be in the list of places your
`pkg-config` looks into. If so you might need to do:
```bash
export PKG_CONFIG_PATH=/usr/local/share/pkgconfig
```
before running `pkg-config`


### Copying to your sources

You can also simply download the [inc](inc) directory of this repository somewhere in your source tree and add it to your include path.
However, note that using this method you will not have `ptl/config.h` configuration header for your platform generated by CMake and will
need to work around it. See [Configuration](#configuration) for details.

## Configuration

When used via CMake (either through `FetchContent` or by installing) PTL has configuration header `ptl/config.h` generated that specifies which non-standard facilities are available on your platform. 

If you do not want to use CMake at all you can either:
1. Provide this header yourself or
2. Define `PTL_NO_CONFIG` no config macro for the compilation to prevent its inclusion.

In either case you can use PTL without any configuration macros defined but it will be limited to standard Posix functionality available everywhere.
You can set the configuration macros manually, if desired, to match your platform. See [`cmake/config.cmake`](cmake/config.cmake) for available macros and their meaning.

## Usage

To access PTL you will need to either include individual `ptl/foo.h` headers (see [Function Mapping](doc/function-mapping.md) for which header provides what functionality) or simply include an umbrella header `ptl/ptl.h`.

Everything in the library is under `namespace ptl`. 




