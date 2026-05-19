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

# ApprovalTests may add /W4 /WX when Clang simulates MSVC; plain clang++ rejects them.
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    get_target_property(_approval_copts ApprovalTests COMPILE_OPTIONS)
    if(_approval_copts)
        list(FILTER _approval_copts EXCLUDE REGEX "^/")
        set_target_properties(ApprovalTests PROPERTIES COMPILE_OPTIONS "${_approval_copts}")
    endif()
endif()

include_directories(include)

enable_testing()

add_library(TVController src/TVController.cpp)
target_include_directories(TVController PUBLIC include)

set(TEST_WORKING_DIR ${CMAKE_SOURCE_DIR}/test)

# Tuner Test
add_executable(TunerTest test/TunerTest.cpp)
target_link_libraries(TunerTest GTest::gtest_main GTest::gmock)

# TVController Test
add_executable(TVControllerTest test/TVControllerTest.cpp)
target_link_libraries(TVControllerTest GTest::gtest_main GTest::gmock TVController)

# Golden Master (pure Google Test file compare)
add_executable(GoldenMasterTest
    test/GoldenMasterTest.cpp
    test/TexttestFixture.cpp
)
target_compile_definitions(GoldenMasterTest PRIVATE
    GOLDEN_DIR="${CMAKE_SOURCE_DIR}/test/golden")
target_link_libraries(GoldenMasterTest GTest::gtest_main TVController)

# Approval Test (ApprovalTests.cpp wrapper over the same fixture)
add_executable(ApprovalTest
    test/ApprovalTest.cpp
    test/TexttestFixture.cpp
)
target_link_libraries(ApprovalTest
    ApprovalTests::ApprovalTests
    GTest::gtest_main
    TVController
)

# Legacy target name kept for existing scripts/docs
add_executable(all_tests ALIAS ApprovalTest)

# TVController Mock Test
add_executable(TVControllerMockTest test/TVControllerMockTest.cpp)
target_link_libraries(TVControllerMockTest GTest::gtest_main GTest::gmock TVController)

# Coverage
target_compile_options(TVControllerTest PRIVATE --coverage)
target_link_options(TVControllerTest PRIVATE --coverage)

include(GoogleTest)
gtest_discover_tests(TunerTest WORKING_DIRECTORY ${TEST_WORKING_DIR})
gtest_discover_tests(TVControllerTest WORKING_DIRECTORY ${TEST_WORKING_DIR})
gtest_discover_tests(TVControllerMockTest WORKING_DIRECTORY ${TEST_WORKING_DIR})
gtest_discover_tests(GoldenMasterTest WORKING_DIRECTORY ${TEST_WORKING_DIR})
gtest_discover_tests(ApprovalTest WORKING_DIRECTORY ${TEST_WORKING_DIR})

# Convenience aggregate target for CI/local runs
add_custom_target(run_all_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS TunerTest TVControllerTest TVControllerMockTest GoldenMasterTest ApprovalTest
)
