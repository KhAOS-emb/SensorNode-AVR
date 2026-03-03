cmake_minimum_required(VERSION 3.16)
project(SensorNode_Tests CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Download & build GoogleTest
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)
FetchContent_MakeAvailable(googletest)

# Bind all .cpp files from src, except for main.cpp and HAL files
file(GLOB_RECURSE SRC_FILES
    "src/app/*.cpp"
    "src/drivers/*.cpp"
)

# Make Test-Executable
file(GLOB_RECURSE TEST_FILES "tests/unit/*.cpp")

add_executable(run_tests
    ${TEST_FILES}
    ${SRC_FILES}
)

# include header files
target_include_directories(run_tests PRIVATE
    src/
    tests/mocks/
)

# Link GoogleTest
target_link_libraries(run_tests
    GTest::gtest_main
    GTest::gmock_main   # GMock für Mock-Objekte
)

# Register & run tests
include(GoogleTest)
gtest_discover_tests(run_tests)