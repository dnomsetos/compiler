cmake_minimum_required(VERSION 3.15)

project(compiler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE COMPILER_SRC
    CONFIGURE_DEPENDS
    src/*.cpp
)

list(FILTER COMPILER_SRC EXCLUDE REGEX "src/main.cpp$")

add_library(compiler_objects OBJECT
    ${COMPILER_SRC}
)

target_include_directories(compiler_objects
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(compiler_objects
    PRIVATE
        -Wall -Wextra -Wpedantic
)

add_executable(compiler
    src/main.cpp
    $<TARGET_OBJECTS:compiler_objects>
)

add_custom_target(run
    COMMAND $<TARGET_FILE:compiler>
    DEPENDS compiler
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running compilier"
)

enable_testing()

add_subdirectory(tests)
