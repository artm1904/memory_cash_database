cmake_minimum_required(VERSION 3.10)
project(database VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/contrib)

include(cmake/cpm.cmake)

CPMAddPackage(
    NAME GOOGLETEST
    GITHUB_REPOSITORY google/googletest
    VERSION 1.16.0
    SOURCE_DIR ${LIB_DIR}/googletest
    OPTIONS 
        "INSTALL_GTEST OFF"
        "gtest_force_shared_crt ON"
)

enable_testing()

# Устанавливаем опции компилятора для нашего проекта
set(PROJECT_COMPILE_OPTIONS)
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    list(APPEND PROJECT_COMPILE_OPTIONS -Wall -Wextra -pedantic)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    list(APPEND PROJECT_COMPILE_OPTIONS -O3)
endif()

add_subdirectory(my_database)
add_subdirectory(tests)
