# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/)

## [Unreleased]

### Changed

-   Update note-c sub-module library to the latest commit.

### Fixed

-   Stabilize I2C communication by adding 1ms delay before every i2c I/O 
    transaction.

## [1.0.2] - 2023-05-22

### Changed

-   Long debug log messages are now not ignored, but printed, up to 255 
    characters.

## [1.0.1] - 2023-05-18

### Fixed

-   Interrupt handling.

## [1.0.0] - 2023-04-06

### Added

-   Basic Notecard driver support over I2C and UART
-   Two basic samples, one for the each of the communication busses, that request
    version string from notecard.
-   Interrupt support for `ATTN_P` pin.
-   Add function for determining available heap memory used by notecard driver.
-   Print logs now remove trailing whitespace, so everything looks nice.

[Unreleased]: https://github.com/IRNAS/irnas-notecard-driver/compare/v1.0.2...HEAD

[1.0.2]: https://github.com/IRNAS/irnas-notecard-driver/compare/v1.0.1...v1.0.2

[1.0.1]: https://github.com/IRNAS/irnas-notecard-driver/compare/v1.0.0...v1.0.1

[1.0.0]: https://github.com/IRNAS/irnas-notecard-driver/compare/6a5696d6b4d6f8aaa269a625594a3d7e93eccd55...v1.0.0
