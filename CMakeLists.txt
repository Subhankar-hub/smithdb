cmake_minimum_required(VERSION 3.25)

# Replaces CMake's default per-configuration flags (Debug: -O0 -g, Release: -O2 -DNDEBUG).
set(CMAKE_USER_MAKE_RULES_OVERRIDE ${CMAKE_CURRENT_LIST_DIR}/cmake/CompilerFlagsOverride.cmake)

project(SmithDB
    VERSION 0.1.0
    DESCRIPTION "A small relational database written from scratch in modern C++"
    LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

get_property(SMITHDB_IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT SMITHDB_IS_MULTI_CONFIG AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type (Debug or Release)" FORCE)
endif()
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release)

option(SMITHDB_BUILD_TESTS "Build the SmithDB unit tests" ON)
option(SMITHDB_ENABLE_SANITIZERS "Build SmithDB with AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(SMITHDB_ENABLE_TSAN "Build SmithDB with ThreadSanitizer" OFF)
option(SMITHDB_ENABLE_LTO "Build SmithDB with link-time optimization" OFF)
option(SMITHDB_ENABLE_CLANG_TIDY "Run clang-tidy while compiling SmithDB targets" OFF)

# Warning flags are applied only to SmithDB's own targets, not to third-party code.
add_library(smithdb_warnings INTERFACE)
if(MSVC)
    target_compile_options(smithdb_warnings INTERFACE /W4 /permissive-)
else()
    target_compile_options(smithdb_warnings INTERFACE
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wsign-conversion)
endif()

# Sanitizer flags propagate from the smithdb library to everything that links it.
add_library(smithdb_sanitizers INTERFACE)
if(SMITHDB_ENABLE_SANITIZERS AND SMITHDB_ENABLE_TSAN)
    message(FATAL_ERROR
        "SMITHDB_ENABLE_SANITIZERS and SMITHDB_ENABLE_TSAN cannot be enabled together")
endif()
if(SMITHDB_ENABLE_SANITIZERS OR SMITHDB_ENABLE_TSAN)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        message(FATAL_ERROR "Sanitizers are only supported with Clang or GCC")
    endif()
    if(SMITHDB_ENABLE_SANITIZERS)
        set(SMITHDB_SANITIZER_FLAGS -fsanitize=address,undefined -fno-omit-frame-pointer)
    else()
        set(SMITHDB_SANITIZER_FLAGS -fsanitize=thread -fno-omit-frame-pointer)
    endif()
    target_compile_options(smithdb_sanitizers INTERFACE ${SMITHDB_SANITIZER_FLAGS})
    target_link_options(smithdb_sanitizers INTERFACE ${SMITHDB_SANITIZER_FLAGS})
endif()

if(SMITHDB_ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT SMITHDB_IPO_SUPPORTED OUTPUT SMITHDB_IPO_OUTPUT LANGUAGES CXX)
    if(NOT SMITHDB_IPO_SUPPORTED)
        message(FATAL_ERROR "SMITHDB_ENABLE_LTO is ON but IPO is not supported: ${SMITHDB_IPO_OUTPUT}")
    endif()
endif()

if(SMITHDB_ENABLE_CLANG_TIDY)
    find_program(SMITHDB_CLANG_TIDY_EXE NAMES clang-tidy REQUIRED)
endif()

# Applies per-target tooling options to SmithDB's own targets only.
function(smithdb_configure_target target)
    if(SMITHDB_ENABLE_LTO)
        set_property(TARGET ${target} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
    if(SMITHDB_ENABLE_CLANG_TIDY)
        set_property(TARGET ${target} PROPERTY CXX_CLANG_TIDY ${SMITHDB_CLANG_TIDY_EXE})
    endif()
endfunction()

# ---------------------------------------------------------------------------
# Core library
# ---------------------------------------------------------------------------
set(SMITHDB_LIBRARY_SOURCES
    src/common/types.cpp
    src/common/config.cpp
    src/storage/page.cpp
    src/storage/disk_manager.cpp
    src/storage/buffer_pool.cpp
    src/storage/heap_table.cpp
    src/record/value.cpp
    src/record/tuple.cpp
    src/record/serializer.cpp
    src/catalog/column.cpp
    src/catalog/schema.cpp
    src/catalog/catalog.cpp
    src/index/btree.cpp)

add_library(smithdb ${SMITHDB_LIBRARY_SOURCES})

target_include_directories(smithdb PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_compile_features(smithdb PUBLIC cxx_std_20)
target_link_libraries(smithdb PRIVATE smithdb_warnings PUBLIC smithdb_sanitizers)
smithdb_configure_target(smithdb)

# ---------------------------------------------------------------------------
# Executables
# ---------------------------------------------------------------------------
add_executable(smithdb_example examples/basic_usage.cpp)
target_link_libraries(smithdb_example PRIVATE smithdb smithdb_warnings)
smithdb_configure_target(smithdb_example)

add_executable(smithdb_inspect tools/db_inspect.cpp)
target_link_libraries(smithdb_inspect PRIVATE smithdb smithdb_warnings)
smithdb_configure_target(smithdb_inspect)

add_executable(smithdb_storage_benchmark benchmarks/storage_benchmark.cpp)
target_link_libraries(smithdb_storage_benchmark PRIVATE smithdb smithdb_warnings)
smithdb_configure_target(smithdb_storage_benchmark)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
set(SMITHDB_TEST_SOURCES
    tests/common/types_test.cpp
    tests/storage/page_test.cpp
    tests/storage/disk_manager_test.cpp
    tests/storage/buffer_pool_test.cpp
    tests/storage/heap_table_test.cpp
    tests/record/value_test.cpp
    tests/record/tuple_test.cpp
    tests/record/serializer_test.cpp
    tests/catalog/schema_test.cpp
    tests/catalog/catalog_test.cpp
    tests/index/btree_test.cpp)

if(SMITHDB_BUILD_TESTS)
    enable_testing()

    find_package(GTest CONFIG QUIET)
    if(NOT GTest_FOUND)
        include(FetchContent)
        FetchContent_Declare(googletest
            URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.tar.gz
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
            SYSTEM)
        set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
        set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googletest)
        # Third-party warnings must not fail builds configured with CMAKE_COMPILE_WARNING_AS_ERROR.
        set_target_properties(gtest gtest_main PROPERTIES COMPILE_WARNING_AS_ERROR OFF)
    endif()

    add_executable(smithdb_tests ${SMITHDB_TEST_SOURCES})
    target_link_libraries(smithdb_tests PRIVATE smithdb smithdb_warnings GTest::gtest_main)
    smithdb_configure_target(smithdb_tests)

    include(GoogleTest)
    gtest_discover_tests(smithdb_tests)
endif()

# ---------------------------------------------------------------------------
# Developer helper targets (never part of the default build)
# ---------------------------------------------------------------------------
add_custom_target(smithdb-example)
add_dependencies(smithdb-example smithdb_example)

if(SMITHDB_BUILD_TESTS)
    add_custom_target(smithdb-test
        COMMAND ${CMAKE_CTEST_COMMAND} --test-dir ${CMAKE_BINARY_DIR} --output-on-failure
        USES_TERMINAL)
    add_dependencies(smithdb-test smithdb_tests)
endif()

file(GLOB_RECURSE SMITHDB_FORMAT_SOURCES CONFIGURE_DEPENDS
    ${PROJECT_SOURCE_DIR}/include/*.hpp
    ${PROJECT_SOURCE_DIR}/src/*.cpp
    ${PROJECT_SOURCE_DIR}/tests/*.cpp
    ${PROJECT_SOURCE_DIR}/examples/*.cpp
    ${PROJECT_SOURCE_DIR}/tools/*.cpp)

find_program(SMITHDB_CLANG_FORMAT_EXE NAMES clang-format)
if(SMITHDB_CLANG_FORMAT_EXE)
    add_custom_target(smithdb-format
        COMMAND ${SMITHDB_CLANG_FORMAT_EXE} -i --style=file ${SMITHDB_FORMAT_SOURCES}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        COMMENT "Formatting SmithDB sources with clang-format"
        VERBATIM)
else()
    add_custom_target(smithdb-format
        COMMAND ${CMAKE_COMMAND} -E echo "smithdb-format: clang-format executable not found"
        COMMAND ${CMAKE_COMMAND} -E false
        VERBATIM)
endif()

list(TRANSFORM SMITHDB_LIBRARY_SOURCES PREPEND ${PROJECT_SOURCE_DIR}/ OUTPUT_VARIABLE SMITHDB_LINT_SOURCES)
list(APPEND SMITHDB_LINT_SOURCES
    ${PROJECT_SOURCE_DIR}/examples/basic_usage.cpp
    ${PROJECT_SOURCE_DIR}/tools/db_inspect.cpp
    ${PROJECT_SOURCE_DIR}/benchmarks/storage_benchmark.cpp)
if(SMITHDB_BUILD_TESTS)
    list(TRANSFORM SMITHDB_TEST_SOURCES PREPEND ${PROJECT_SOURCE_DIR}/ OUTPUT_VARIABLE SMITHDB_LINT_TEST_SOURCES)
    list(APPEND SMITHDB_LINT_SOURCES ${SMITHDB_LINT_TEST_SOURCES})
endif()

find_program(SMITHDB_CLANG_TIDY_LINT_EXE NAMES clang-tidy)
if(SMITHDB_CLANG_TIDY_LINT_EXE)
    add_custom_target(smithdb-lint
        COMMAND ${SMITHDB_CLANG_TIDY_LINT_EXE} -p ${CMAKE_BINARY_DIR} ${SMITHDB_LINT_SOURCES}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        COMMENT "Running clang-tidy on SmithDB sources"
        VERBATIM)
else()
    add_custom_target(smithdb-lint
        COMMAND ${CMAKE_COMMAND} -E echo "smithdb-lint: clang-tidy executable not found"
        COMMAND ${CMAKE_COMMAND} -E false
        VERBATIM)
endif()
