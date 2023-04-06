# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/)

## [Unreleased]

## Added

- Basic Notecard driver support over I2C and UART
- Two basic samples, one for the each of the communication busses, that request
  version string from notecard.
- Interrupt support for `ATTN_P` pin.
- Add function for determining available heap memory used by notecard driver.
- Print logs now remove trailing whitespace, so everything looks nice.
