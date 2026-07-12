cmake_minimum_required(VERSION 3.24)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2
)
FetchContent_MakeAvailable(googletest)

add_executable(RecoveryEngineTests
    ParserProbeTests.cpp
    AhoCorasickTests.cpp
    RaidReconstructorTests.cpp
)

target_include_directories(RecoveryEngineTests PRIVATE
    ${CMAKE_SOURCE_DIR}/src/RecoveryEngine/include
)

target_link_libraries(RecoveryEngineTests PRIVATE
    RecoveryEngine
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(RecoveryEngineTests)
