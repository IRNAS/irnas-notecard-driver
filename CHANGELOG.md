# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/)

## [Unreleased]

### Changed

-   Update CI and `pre-commit` files in the project to the latest version.
-   Run `pre-commit` on the whole project
-   Update project to use NCS v3.0.1.

### Fixed

-   Fix the disable logging logic in `notecard_is_present` to not assert if `i2c_nrfx_twim` is being used instead of `i2c_nrfx_twi`.

## [1.4.2] - 2023-11-30

### Changed

-   Add back `log_level_set` in `notecard_is_present` function, as the the compiler errors were
    resolved.

## [1.4.1] - 2023-11-29

### Changed

-   Temporary remove `log_level_set` in `notecard_is_present` function, until
    the compiler errors are resolved.

## [1.4.0] - 2023-11-22

### Added

-   Improve notecard logging interface. Log levels from the note-c library are
    now correctly translated to the Zephyr's log levels.
-   Update CI infrastructure and migrate to the trunk-based development model.

### Changed

-   Change name of the logging module from `note` to `notecard`, since `note`
    was too vague (it could be mistakenly related to the `note-c`).

-   Separate compilation of the `note-c` lib from Notecard driver. `note-c` is
    now included and compiled via newly added Kconfig symbol,
    `CONFIG_NOTE_C_LIB.` `CONFIG_NOTECARD` always selects it, as it needs it.
    Refer to the `README.md` for more info.

### Removed

-   Remove 1ms delays before every i2c I/O transaction, since they are not
    needed any more with the current note-c library.

## [1.3.0] - 2023-11-16

### Added

-   `notecard_is_present()` API.

### Changed

-   Switch to using Blues version of note-c library. This will means that that
    large binary payload feature can be used.

## [1.2.0] - 2023-07-27

### Changed

-   Update note-c submodule to v0.1.1 tag with longer 5 second retries which
    solve web.post issues.

## [1.1.1] - 2023-07-12

### Changed

-   Switch to using Irnas's fork of note-c library so you are still using
    pre-non-turboIO logic, with shorter retryies.

## [1.1.0] - 2023-07-03

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

[Unreleased]: https://github.com/IRNAS/irnas-notecard-driver/compare/v1.4.2...HEAD

[1.4.2]: https://github.com/IRNAS/irnas-notecard-driver/compare/v1.4.1...v1.4.2

[1.4.1]: https://github.com/IRNAS/irnas-notecard-driver/compare/v1.4.0...v1.4.1

[1.4.0]: https://github.com/IRNAS/irnas-notecard-driver/compare/v1.3.0...v1.4.0

[1.3.0]: https://github.com/IRNAS/irnas-notecard-driver/compare/v1.2.0...v1.3.0

[1.2.0]: https://github.com/IRNAS/irnas-notecard-driver/compare/v1.1.1...v1.2.0

[1.1.1]: https://github.com/IRNAS/irnas-notecard-driver/compare/v1.1.0...v1.1.1

[1.1.0]: https://github.com/IRNAS/irnas-notecard-driver/compare/v1.0.2...v1.1.0

[1.0.2]: https://github.com/IRNAS/irnas-notecard-driver/compare/v1.0.1...v1.0.2

[1.0.1]: https://github.com/IRNAS/irnas-notecard-driver/compare/v1.0.0...v1.0.1

[1.0.0]: https://github.com/IRNAS/irnas-notecard-driver/compare/6a5696d6b4d6f8aaa269a625594a3d7e93eccd55...v1.0.0
