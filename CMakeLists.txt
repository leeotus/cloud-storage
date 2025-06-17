cmake_minimum_required(VERSION 3.10...3.24)
project(filekeeper LANGUAGES C CXX VERSION 1.0)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CLOUD_STORAGE_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/inc")

add_subdirectory(src)
option(BUILD_FILEKEEPER_APP "also build application for this project" ON)
if(${BUILD_FILEKEEPER_APP})
  add_subdirectory(app)
endif()

option(BUILD_FILEKEEPER_TEST "also build tests for this library" ON)
if(${BUILD_FILEKEEPER_TEST})
  add_subdirectory(test)
endif()
