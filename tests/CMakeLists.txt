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
    $<TARGET_OBJECTS:compiler_objects>
)

target_include_directories(tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
        /test_utilities
)

target_link_libraries(tests
    PRIVATE
        compiler_headers
        GTest::gtest_main
        ${LLVM_LIBS}
)

target_compile_options(tests
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        $<$<BOOL:${ENABLE_SANITIZERS}>:-fsanitize=address,undefined>
)

target_link_options(tests
    PRIVATE
        $<$<BOOL:${ENABLE_SANITIZERS}>:-fsanitize=address,undefined>
)

message(STATUS "TEST_DATA_DIR: ${CMAKE_SOURCE_DIR}/tests/test_files/")

set(HELPERS_OBJ "${CMAKE_CURRENT_BINARY_DIR}/helpers.o")
add_custom_command(
    OUTPUT "${HELPERS_OBJ}"
    COMMAND ${CMAKE_C_COMPILER} -c "${CMAKE_SOURCE_DIR}/tests/test_files/helpers.c" -o "${HELPERS_OBJ}"
    DEPENDS "${CMAKE_SOURCE_DIR}/tests/test_files/helpers.c"
    COMMENT "Compiling helpers.c"
)
add_custom_target(helpers_obj DEPENDS "${HELPERS_OBJ}")
add_dependencies(tests helpers_obj)

target_compile_definitions(tests PRIVATE
    PROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
    TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/tests/test_files/"
    TEST_HELPERS_OBJ="${HELPERS_OBJ}"
)

include(GoogleTest)
gtest_discover_tests(
    tests
    NO_PRETTY_VALUES
)

add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running all unit tests"
)

find_program(VALGRIND_EXECUTABLE valgrind)

if(VALGRIND_EXECUTABLE)
    add_custom_target(valgrind_cachegrind
        COMMAND ${VALGRIND_EXECUTABLE} --tool=cachegrind --cache-sim=yes --branch-sim=yes $<TARGET_FILE:tests>
        DEPENDS tests
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Running tests under Valgrind Cachegrind"
    )

    add_custom_target(valgrind_memcheck
        COMMAND ${VALGRIND_EXECUTABLE} --tool=memcheck --leak-check=full --track-origins=yes $<TARGET_FILE:tests>
        DEPENDS tests
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Running tests under Valgrind Memcheck"
    )
endif()
