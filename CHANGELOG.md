# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),

## Unreleased

## [1.2] - 2024-05-02

## Added
- Support for illumos derived systems

## Fixed
- Incorrect handling of IPV6_MULTICAST_LOOP socket option on some platforms

## [1.1] - 2024-04-30

## Added
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
