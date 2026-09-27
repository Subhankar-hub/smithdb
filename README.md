# SmithDB

A small relational database implemented from scratch in modern C++ for learning database
internals and systems programming.

## Overview

SmithDB is built bottom-up, starting from the physical storage layer. The goal is a clear,
well-documented implementation of the classic relational database architecture rather than a
production system. It is written in C++20 using only the standard library; GoogleTest is used for
tests.

## Architecture

```text
Application / future SQL layer
        ↓
      Catalog
        ↓
      Records
        ↓
 HeapTable / Index
        ↓
    BufferPool
        ↓
    DiskManager
        ↓
   database file
```

See [docs/architecture.md](docs/architecture.md) for details,
[docs/file_format.md](docs/file_format.md) for the on-disk format, and
[docs/invariants.md](docs/invariants.md) for the invariants each subsystem maintains.

## Current Status

Implemented:

- **DiskManager**: opens or creates a database file, allocates pages sequentially, and reads
  and writes complete 4096-byte pages that persist across restarts. Page IDs and file size are
  validated.
- **Page**: a fixed-size 4096-byte buffer.

Early groundwork, not yet a complete phase:

- **BufferPool**: a minimal fixed-size page cache with pinning, dirty tracking, and simple
  first-unpinned eviction (no LRU, no concurrency).
- **Value / Tuple**: in-memory representations of NULL, INTEGER, FLOAT, and TEXT values and rows.
- **Schema / Catalog**: in-memory table definitions with validation. Nothing is persisted.

Interface only (operations throw `DatabaseError` with `ErrorCode::NotImplemented`):

- `HeapTable`, `Serializer`, `BTree`.

There is no SQL parser, query execution, WAL, transaction support, or concurrency.

## Build

Requirements: a C++20 compiler (Clang or GCC), CMake 3.25 or newer, and Ninja. GoogleTest is
used from the system if CMake can find it; otherwise it is downloaded automatically at configure
time.

The recommended way to build is with the CMake presets described in
[Development Toolchain](#development-toolchain). A plain configure also works:

```bash
cmake -S . -B build -G Ninja                              # Debug (default)
cmake --build build

cmake -S . -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-release
```

Select a compiler with the standard CMake variables, for example
`-DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++` or
`-DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++`.

CMake options:

| Option                      | Default | Description                                         |
|-----------------------------|---------|-----------------------------------------------------|
| `SMITHDB_BUILD_TESTS`       | `ON`    | Build the unit tests                                |
| `SMITHDB_ENABLE_SANITIZERS` | `OFF`   | AddressSanitizer + UndefinedBehaviorSanitizer       |
| `SMITHDB_ENABLE_TSAN`       | `OFF`   | ThreadSanitizer (cannot be combined with ASan/UBSan) |
| `SMITHDB_ENABLE_LTO`        | `OFF`   | Link-time optimization through CMake's IPO support  |
| `SMITHDB_ENABLE_CLANG_TIDY` | `OFF`   | Run clang-tidy while compiling SmithDB targets      |

Build types: `Debug` uses `-O0 -g`; `Release` uses `-O2 -DNDEBUG`.

Build targets:

| Target                      | Description                                    |
|-----------------------------|------------------------------------------------|
| `smithdb`                   | Core library                                   |
| `smithdb_example`           | Page write/read demo (`examples/basic_usage.cpp`) |
| `smithdb_inspect`           | Database file inspector (`tools/db_inspect.cpp`)  |
| `smithdb_storage_benchmark` | Sequential page I/O timing                     |
| `smithdb_tests`             | Unit tests                                     |

Helper targets (never built by default):

| Target            | Description                                                          |
|-------------------|----------------------------------------------------------------------|
| `smithdb-format`  | Run `clang-format -i` on `include/`, `src/`, `tests/`, `examples/`, `tools/` |
| `smithdb-lint`    | Run `clang-tidy` on SmithDB sources using `compile_commands.json`    |
| `smithdb-test`    | Build the tests and run `ctest --output-on-failure`                  |
| `smithdb-example` | Build `smithdb_example`                                              |

```bash
./build/smithdb_example demo.db
./build/smithdb_inspect demo.db
```

## Run Tests

```bash
ctest --test-dir build --output-on-failure
```

## Development Toolchain

| Role               | Tool          |
|--------------------|---------------|
| Primary compiler   | Clang/LLVM    |
| Secondary compiler | GCC           |
| Build system       | CMake         |
| Generator          | Ninja         |
| Debugger           | LLDB          |
| Static analysis    | clang-tidy    |
| Formatting         | clang-format  |

The code is standard C++20 with no compiler-specific extensions and must build unchanged with
both `clang++` and `g++`. The presets use the unversioned `clang`/`clang++` and `gcc`/`g++`
executables found on `PATH`. Build directories are created under `build/<preset>`.

### Debug development

```bash
cmake --preset smithdb-clang-debug
cmake --build --preset smithdb-clang-debug
ctest --preset smithdb-clang-debug
```

Debugging with LLDB:

```bash
lldb ./build/smithdb-clang-debug/smithdb_example -- demo.db
```

```text
(lldb) breakpoint set --name main     # or: breakpoint set --file disk_manager.cpp --line 42
(lldb) run
(lldb) next                           # step over
(lldb) step                           # step into
(lldb) continue
(lldb) frame variable                 # locals in the current frame
(lldb) bt                             # backtrace
```

### Sanitizer development

AddressSanitizer and UndefinedBehaviorSanitizer are applied to the library, executables, and tests:

```bash
cmake --preset smithdb-clang-asan
cmake --build --preset smithdb-clang-asan
ctest --preset smithdb-clang-asan
```

ThreadSanitizer is available through `-DSMITHDB_ENABLE_TSAN=ON` in a separate build directory. It
becomes useful once SmithDB introduces concurrency; it cannot be enabled together with
`SMITHDB_ENABLE_SANITIZERS`.

### GCC portability validation

```bash
cmake --preset smithdb-gcc-debug
cmake --build --preset smithdb-gcc-debug
ctest --preset smithdb-gcc-debug
```

### Static analysis and formatting

```bash
cmake --build --preset smithdb-clang-debug --target smithdb-lint     # clang-tidy, uses .clang-tidy
cmake --build --preset smithdb-clang-debug --target smithdb-format   # clang-format -i, uses .clang-format
```

Alternatively configure with `-DSMITHDB_ENABLE_CLANG_TIDY=ON` to run clang-tidy during every
compile. Formatting never runs as part of an ordinary build.

### LLVM IR inspection

Generating IR and assembly is an optional manual workflow, not part of the build:

```bash
# LLVM IR
clang++ -std=c++20 -O2 -Iinclude -S -emit-llvm src/storage/page.cpp -o page.ll

# Assembly
clang++ -std=c++20 -O2 -Iinclude -S src/storage/page.cpp -o page.s

# Object files produced by a build
llvm-objdump -d --demangle build/smithdb-clang-debug/CMakeFiles/smithdb.dir/src/storage/page.cpp.o
llvm-readelf --sections --symbols build/smithdb-clang-debug/smithdb_example
llvm-nm --demangle build/smithdb-clang-debug/libsmithdb.a
```

### Release and performance experiments

```bash
cmake --preset smithdb-clang-release
cmake --build --preset smithdb-clang-release
ctest --preset smithdb-clang-release
```

The Release configuration uses `-O2` and is portable across machines of the same architecture. Link-time
optimization is available with `-DSMITHDB_ENABLE_LTO=ON`. Architecture-specific flags such as
`-march=native` and `-mtune=native` are only for local performance experiments and should be
passed explicitly in a separate build directory, for example:

```bash
cmake -S . -B build/native -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_CXX_FLAGS="-march=native -mtune=native"
```

## Project Structure

```text
include/smithdb/   Public headers
  common/          Identifiers, constants, errors, configuration
  storage/         Page, DiskManager, BufferPool, HeapTable
  record/          Value, Tuple, Serializer
  catalog/         Column, Schema, Catalog
  index/           B+ tree interface
src/               Implementations, mirroring include/
tests/             GoogleTest unit tests, mirroring include/
examples/          Small programs using the library
tools/             Diagnostic utilities
benchmarks/        Performance experiments
docs/              Architecture, file format, invariants
```

## Design Principles

- Each layer has one responsibility and depends only on layers below it.
- `DiskManager` knows about files and pages, not SQL. `Page` knows about bytes, not tables.
  `Catalog` knows about schemas, not file I/O.
- Logical data (`Value`, `Tuple`, `Schema`) is separate from physical storage (`Page`, file
  layout). Records are serialized explicitly, never by copying C++ object memory to disk.
- Every subsystem documents its invariants.
- RAII, const correctness, and standard library types (`std::span`, `std::optional`,
  `std::variant`, `std::filesystem`); no global mutable state or singletons.
- Features are added only when needed, not speculatively.

## Roadmap

| Phase    | Component              | Status      |
|----------|------------------------|-------------|
| Phase 1  | Disk Manager           | Implemented |
| Phase 2  | Page / Slotted Pages   | Planned     |
| Phase 3  | Records                | Planned     |
| Phase 4  | Buffer Pool            | Planned     |
| Phase 5  | Heap Table             | Planned     |
| Phase 6  | Catalog                | Planned     |
| Phase 7  | Sequential Scan        | Planned     |
| Phase 8  | SQL Parser             | Planned     |
| Phase 9  | Query Execution        | Planned     |
| Phase 10 | B+ Tree                | Planned     |
| Phase 11 | WAL / Recovery         | Planned     |
| Phase 12 | Transactions           | Planned     |
| Phase 13 | Concurrency            | Planned     |

## License

MIT. See [LICENSE](LICENSE).
