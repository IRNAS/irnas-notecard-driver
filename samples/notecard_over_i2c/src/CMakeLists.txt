# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
# create compile_commands.json for clang
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(basic)

# We always want colored gcc output
zephyr_compile_options(-fdiagnostics-color=always)

# Set CMake path variables for convenience
set(PROJECT_DIR ../..)
set(NRF_SDK_DIR ${ZEPHYR_BASE}/../nrf)

file(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})

# add include directories from application
target_include_directories(app PRIVATE ${PROJECT_DIR}/app/src)
