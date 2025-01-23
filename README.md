# PTL

[![Language](https://img.shields.io/badge/language-C++-blue.svg)](https://isocpp.org/)
[![Standard](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization)
[![License](https://img.shields.io/badge/license-BSD-brightgreen.svg)](https://opensource.org/licenses/BSD-3-Clause)


A C++ library for Posix and related calls.

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
* Supports both error code returns (`std::error_code`, `boost::error_code` or any error code class you might want to use!) and exceptions _without code duplication_. See [Error Handling](doc/usage.md#error-handling) for details.
* RAII everywhere there is do/undo semantics.
* Free functions that take conceptual arguments rather than class methods whenever practical. For example, using this library you can say `duplicate(STDOUT_FILENO)` or `duplicate(stdout)` or `duplicate(an_instance_of_file_descriptor_class)` and they all will do the same thing: duplicate the file descriptor and produce a new instance of file descriptor class. The `duplicate` function takes anything that matches a "file like" concept. This way you can mix and match PTL with plain Posix code as desired. That is, PTL is not all or nothing proposition. You can even make PTL work with your own classes if you have them by matching them to a concept (via traits specialization usually).

## Extensions

In addition to portable Posix functionality PTL also includes support for some common extensions. If you use PTL via CMake or build and install it the extensions supported on your platform will be automatically detected and enabled. If you simply grab PTL headers then you will need to manually set various enabling macros to 1 in order to enable them. See [Configuration](doc/building.md#configuration) for more details.

## Android compatibility

PTL can be used on Android. However, note that some functionality may not be available based on what Posix calls are provided on given Android API level. Generally, all things should work above API level 30. Also note that Android often deviates from exact Posix semantics of many calls even if the calls themselves are available. In some cases, where the deviation can affect safety (e.g. `gethostname` not null terminating truncated output) or is especially annoying (`getpwnam_r` producing hard error when username is not found) PTL 'fixes' the issue. In other cases (like `posix_spawn` not reporting error when executable is not found) you will need to deal with differences in your own code. 

## Emscripten/Wasm compatibility

PTL can be used on Wasm (at least with Emscripten, possibly with other toolchains too). Note that Emscripten often declares many Posix calls it doesn't actually implement in its headers and makes them either not link or fail at runtime. PTL cannot detect this during configuration and thus you will need to deal with such cases in your own code. 

## Win32/MinGW compatibility

PTL can be used on Win32/MinGW but it only exposes very limited Posix-compatibility functionality provided be these environments. If you need a full portability between Unix and Windows you should look for a portable library of some sort.

Note that PTL is fully supported on Cygwin as it is mostly a "normal" Posix-like environment.

## Usage

See [Usage guide](doc/usage.md) (work in progress).

## Building/Integrating

Quickest CMake method is given below. For more details and other methods see [Integration Guide](doc/building.md)

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

