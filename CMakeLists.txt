cmake_minimum_required(VERSION 3.14)
project(bestreviewer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.16.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    TLS_VERIFY OFF
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
    ApprovalTests
    GIT_REPOSITORY https://github.com/approvals/ApprovalTests.cpp.git
    GIT_TAG v.10.13.0
)
FetchContent_MakeAvailable(ApprovalTests)

include_directories(include)

enable_testing()

add_library(TVController src/TVController.cpp)
target_include_directories(TVController PUBLIC include)

# Tuner Test
add_executable(TunerTest test/TunerTest.cpp)
target_link_libraries(TunerTest GTest::gtest_main GTest::gmock)

# TVController Test
add_executable(TVControllerTest test/TVControllerTest.cpp)
target_link_libraries(TVControllerTest GTest::gtest_main GTest::gmock TVController)

# Approval Test
add_executable(all_tests test/ApprovalTest.cpp)
target_include_directories(all_tests PUBLIC include)
target_link_libraries(all_tests
    ApprovalTests::ApprovalTests
    GTest::gtest_main
    GTest::gmock
    TVController
)

# TVController Mock Test
add_executable(TVControllerMockTest test/TVControllerMockTest.cpp)
target_link_libraries(TVControllerMockTest GTest::gtest_main GTest::gmock TVController)

# Coverage
target_compile_options(TVControllerTest PRIVATE --coverage)
target_link_options(TVControllerTest PRIVATE --coverage)

include(GoogleTest)
gtest_discover_tests(TunerTest)
gtest_discover_tests(TVControllerTest)
gtest_discover_tests(TVControllerMockTest)
gtest_discover_tests(all_tests)
