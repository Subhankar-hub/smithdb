cmake_minimum_required(VERSION 3.25)

project(SmithDB
    VERSION 0.1.0
    DESCRIPTION "A small relational database written from scratch in modern C++"
    LANGUAGES CXX)

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

# ---------------------------------------------------------------------------
# Core library
# ---------------------------------------------------------------------------
add_library(smithdb
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

target_include_directories(smithdb PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_compile_features(smithdb PUBLIC cxx_std_20)
target_link_libraries(smithdb PRIVATE smithdb_warnings)

# ---------------------------------------------------------------------------
# Executables
# ---------------------------------------------------------------------------
add_executable(smithdb_example examples/basic_usage.cpp)
target_link_libraries(smithdb_example PRIVATE smithdb smithdb_warnings)

add_executable(smithdb_inspect tools/db_inspect.cpp)
target_link_libraries(smithdb_inspect PRIVATE smithdb smithdb_warnings)

add_executable(smithdb_storage_benchmark benchmarks/storage_benchmark.cpp)
target_link_libraries(smithdb_storage_benchmark PRIVATE smithdb smithdb_warnings)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
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
    endif()

    add_executable(smithdb_tests
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
    target_link_libraries(smithdb_tests PRIVATE smithdb smithdb_warnings GTest::gtest_main)

    include(GoogleTest)
    gtest_discover_tests(smithdb_tests)
endif()
