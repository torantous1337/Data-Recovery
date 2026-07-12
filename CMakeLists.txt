cmake_minimum_required(VERSION 3.24)
project(DataRecovery LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTS "Build unit tests" ON)

if (MSVC)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS WIN32_LEAN_AND_MEAN NOMINMAX)
endif()

add_subdirectory(src/RecoveryEngine)

if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests/RecoveryEngineTests)
endif()
