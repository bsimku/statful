cmake_minimum_required(VERSION 3.4)

project(statbar LANGUAGES C)

find_package(PkgConfig REQUIRED)

pkg_check_modules(PULSE REQUIRED IMPORTED_TARGET libpulse)

add_subdirectory(src)
