cmake_minimum_required(VERSION 3.5)

project(statbar LANGUAGES C)

find_package(PkgConfig REQUIRED)

pkg_check_modules(PULSE REQUIRED IMPORTED_TARGET libpulse)
pkg_check_modules(LUAJIT REQUIRED IMPORTED_TARGET luajit)

add_subdirectory(src)
