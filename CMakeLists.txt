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

add_library(compiler_headers INTERFACE)

target_include_directories(compiler_headers
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(compiler_objects OBJECT
    ${COMPILER_SRC}
)

target_link_libraries(compiler_objects
    PUBLIC compiler_headers
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(compiler_objects
        PRIVATE
            -Wall -Wextra -Wpedantic -fsanitize=address,undefined
    )
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(compiler_objects
        PRIVATE
            -Wall -Wextra -Wpedantic
    )
endif()

add_executable(compiler
    src/main.cpp
    $<TARGET_OBJECTS:compiler_objects>
)

target_link_libraries(compiler
    PRIVATE compiler_headers
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(compiler PRIVATE -fsanitize=address,undefined)
    target_link_options(compiler PRIVATE -fsanitize=address,undefined)
endif()

enable_testing()

add_subdirectory(tests)
