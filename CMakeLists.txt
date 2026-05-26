cmake_minimum_required(VERSION 3.15)

project(compiler LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
llvm_map_components_to_libnames(LLVM_LIBS
    core
    support
    target
    MC
    passes
    TargetParser
    native
)

message(STATUS "llvm libs: " ${LLVM_LIBS})

execute_process(
    COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=libstdc++exp.a
    OUTPUT_VARIABLE STDCXX_EXP_PATH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
message(STATUS "libstdc++exp path: ${STDCXX_EXP_PATH}")

file(GLOB_RECURSE COMPILER_SRC
    CONFIGURE_DEPENDS
    src/*.cpp
)
list(FILTER COMPILER_SRC EXCLUDE REGEX "src/main.cpp$")

add_library(compiler_headers INTERFACE)
target_include_directories(compiler_headers
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${LLVM_INCLUDE_DIRS}
)
target_compile_definitions(compiler_headers
    INTERFACE
        ${LLVM_DEFINITIONS}
)

add_library(compiler_objects OBJECT
    ${COMPILER_SRC}
)
target_link_libraries(compiler_objects
    PUBLIC
        compiler_headers
        ${LLVM_LIBS}
        ${STDCXX_EXP_PATH}
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Debug build with sanitizers")
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
    PRIVATE
        compiler_headers
        ${LLVM_LIBS}
        ${STDCXX_EXP_PATH}
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(compiler PRIVATE -fsanitize=address,undefined)
    target_link_options(compiler PRIVATE -fsanitize=address,undefined)
endif()

enable_testing()
add_subdirectory(tests)
