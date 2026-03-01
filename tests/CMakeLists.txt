cmake_minimum_required(VERSION 3.15)

include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.15.0
)

set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

file(GLOB_RECURSE TEST_SRC
    CONFIGURE_DEPENDS
    *.cpp
)

add_executable(tests
    ${TEST_SRC}
)

target_include_directories(tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(tests
    PRIVATE
        GTest::gtest_main
        $<TARGET_OBJECTS:compiler_objects>
)

target_compile_options(tests
    PRIVATE
        -Wall -Wextra -Wpedantic
)

include(GoogleTest)
gtest_discover_tests(tests)

add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running all unit tests"
)
