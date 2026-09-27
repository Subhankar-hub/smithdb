# SmithDB Architecture

SmithDB is organized as a stack of layers. Each layer depends only on the layers below it and
communicates through narrow interfaces.

```text
SmithDB (application / future SQL layer)
  ↓
Catalog         table schemas
  ↓
Record layer    values, tuples, serialization
  ↓
Heap / Index    record storage and lookup by RID or key
  ↓
Buffer Pool     in-memory cache of pages
  ↓
Disk Manager    page-granular file I/O
  ↓
database file
```

## Layers

| Layer        | Code                                      | Status         |
|--------------|-------------------------------------------|----------------|
| Disk Manager | `storage/disk_manager.hpp`                | Implemented    |
| Page         | `storage/page.hpp`                        | Implemented    |
| Buffer Pool  | `storage/buffer_pool.hpp`                 | Minimal        |
| Heap Table   | `storage/heap_table.hpp`                  | Interface only |
| Index        | `index/btree.hpp`                         | Interface only |
| Record       | `record/value.hpp`, `record/tuple.hpp`    | In-memory only |
| Serializer   | `record/serializer.hpp`                   | Interface only |
| Catalog      | `catalog/schema.hpp`, `catalog/catalog.hpp` | In-memory only |

"Interface only" components throw `DatabaseError` with `ErrorCode::NotImplemented`.

### Disk Manager

Owns the database file. Allocates page IDs sequentially and reads/writes complete
`PAGE_SIZE` pages at offset `page_id * PAGE_SIZE`. Knows nothing about records, tables, or SQL.

### Page

A fixed-size in-memory byte buffer of exactly `PAGE_SIZE` bytes. It does not interpret its
contents; higher layers decide what the bytes mean.

### Buffer Pool

Holds a fixed number of page frames on top of the Disk Manager. Callers pin a page with
`fetch_page`, and release it with `unpin_page`, marking it dirty if modified. When no frame is
free, the first unpinned frame is reused and written back first if dirty. There is no LRU
policy and no thread safety.

### Heap Table and Index

Planned consumers of the Buffer Pool. `HeapTable` will store records in slotted pages addressed
by `RID`; `BTree` will map keys to `RID`s. Only their interfaces exist today.

### Record layer

`Value` is a logical value (NULL, INTEGER, FLOAT, TEXT) backed by `std::variant`. `Tuple` is an
ordered list of values. `Serializer` will define the explicit on-disk encoding of tuples; records
are never written by copying C++ object memory.

### Catalog

An in-memory registry of `Schema`s keyed by table name. Catalog persistence is planned for a
later phase.

## Dependency rules

1. `DiskManager` knows about files and pages, not SQL.
2. `Page` knows about bytes, not tables.
3. `Catalog` knows about schemas and tables, not raw file I/O.
4. Records are never dumped to disk with `reinterpret_cast` or `memcpy` of C++ objects.
5. Logical data representation (`Value`, `Tuple`, `Schema`) is kept separate from physical
   storage representation (`Page`, file layout).
6. Every subsystem documents its invariants (see [invariants.md](invariants.md)).
7. There is no query parser or SQL executor yet.

## Error handling

Operations report success by returning normally and failure by throwing
`smithdb::DatabaseError`, which carries an `ErrorCode` (see `common/status.hpp`). Expected
"absent" results use `std::optional` or `nullptr` instead of exceptions.
