# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),

## Unreleased

## [1.5] - 2025-01-23

### Added
- Ability for the user to override format library detection

## [1.4] - 2025-01-23

### Fixed
- `std::format` presence is now detected better on libc++
- Various fixes to to tests. In particular tests now run on Emscripten 

## [1.3] - 2024-10-02

### Fixed
- Compilation break when using with `fmt` library version 11

## [1.2] - 2024-05-02

### Added
- Support for illumos derived systems

### Fixed
- Incorrect handling of IPV6_MULTICAST_LOOP socket option on some platforms

## [1.1] - 2024-04-30

### Added
- Support for OpenBSD
- Support for Alpine Linux and musl libc

## [1.0] - 2024-04-20

### Fixed
- Build break with GCC 14
- Build Break with Visual Studio 17.9

### Changed
- Unit tests now use Doctest instead of Catch2

## [0.6] - 2023-10-17
### Fixed
- Portability fixes for usage with fmt lib

## [0.5] - 2023-08-02

### Added
- Support for `getgroups`/`setgroups`

## [0.4] - 2023-07-25

### Added
- Additional socket calls: `bind`, `recvxxx`, `sendxx`, `getsockname`

## [0.3] - 2023-07-20

### Added
- Initial support for socket APIs
- Some helpful error handling utilities

### Changed
- Cleaned up error processing machinery

## [0.2] - 2023-07-11

### Added
- Initial version

[0.2]: https://github.com/gershnik/ptl/releases/v0.2
[0.3]: https://github.com/gershnik/ptl/releases/v0.3
[0.4]: https://github.com/gershnik/ptl/releases/v0.4
[0.5]: https://github.com/gershnik/ptl/releases/v0.5
[0.6]: https://github.com/gershnik/ptl/releases/v0.6
[1.0]: https://github.com/gershnik/ptl/releases/v1.0
[1.1]: https://github.com/gershnik/ptl/releases/v1.1
[1.2]: https://github.com/gershnik/ptl/releases/v1.2
[1.3]: https://github.com/gershnik/ptl/releases/v1.3
[1.4]: https://github.com/gershnik/ptl/releases/v1.4
[1.5]: https://github.com/gershnik/ptl/releases/v1.5
