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

Requirements: a C++20 compiler (GCC or Clang), CMake 3.25 or newer, and Ninja. GoogleTest is
used from the system if CMake can find it; otherwise it is downloaded automatically at configure
time.

```bash
cmake -S . -B build -G Ninja                              # Debug (default)
cmake --build build

cmake -S . -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-release
```

Pass `-DSMITHDB_BUILD_TESTS=OFF` to skip building the tests.

Build targets:

| Target                      | Description                                    |
|-----------------------------|------------------------------------------------|
| `smithdb`                   | Core library                                   |
| `smithdb_example`           | Page write/read demo (`examples/basic_usage.cpp`) |
| `smithdb_inspect`           | Database file inspector (`tools/db_inspect.cpp`)  |
| `smithdb_storage_benchmark` | Sequential page I/O timing                     |
| `smithdb_tests`             | Unit tests                                     |

```bash
./build/smithdb_example demo.db
./build/smithdb_inspect demo.db
```

## Run Tests

```bash
ctest --test-dir build --output-on-failure
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
