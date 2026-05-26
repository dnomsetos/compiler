cmake_minimum_required(VERSION 3.15)

project(compiler LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_SANITIZERS "Enable ASan/UBSan" ON)
option(ENABLE_VALGRIND "Build without sanitizers for Valgrind runs" OFF)

if(ENABLE_VALGRIND)
    set(ENABLE_SANITIZERS OFF CACHE BOOL "Enable ASan/UBSan" FORCE)
endif()

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

message(STATUS "llvm libs: ${LLVM_LIBS}")

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

add_library(compiler_objects OBJECT ${COMPILER_SRC})
target_link_libraries(compiler_objects
    PUBLIC
        compiler_headers
        ${LLVM_LIBS}
        ${STDCXX_EXP_PATH}
)

target_compile_options(compiler_objects
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        $<$<BOOL:${ENABLE_SANITIZERS}>:-fsanitize=address,undefined>
)

target_link_options(compiler_objects
    PRIVATE
        $<$<BOOL:${ENABLE_SANITIZERS}>:-fsanitize=address,undefined>
)

if(ENABLE_SANITIZERS)
    message(STATUS "Sanitizers enabled")
else()
    message(STATUS "Sanitizers disabled")
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

target_compile_options(compiler
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        $<$<BOOL:${ENABLE_SANITIZERS}>:-fsanitize=address,undefined>
)

target_link_options(compiler
    PRIVATE
        $<$<BOOL:${ENABLE_SANITIZERS}>:-fsanitize=address,undefined>
)

enable_testing()
add_subdirectory(tests)
