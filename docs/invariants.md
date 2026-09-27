# SmithDB Invariants

An invariant is a property that must hold whenever control is outside a subsystem's methods.
Every subsystem documents its invariants here and in its header. Code changes that alter an
invariant must update this file.

## Global

- `PAGE_SIZE` is fixed at 4096 bytes and defined only in `common/constants.hpp`.
- Page IDs identify physical pages: page `N` is stored at byte offset `N * PAGE_SIZE`.
- An `RID` identifies a (page, slot) pair.

## Page

- A `Page` always owns exactly `PAGE_SIZE` bytes (checked with `static_assert`).
- A newly constructed or `reset()` page is entirely zero.
- A `Page` does not interpret its bytes.

## DiskManager

- `DiskManager` reads and writes complete pages only.
- The database file size is always `page_count() * PAGE_SIZE`.
- Page IDs are allocated sequentially from 0 and are never reused.
- A newly allocated page is zero-filled on disk.
- Reading or writing a page ID `>= page_count()` fails with `ErrorCode::OutOfRange`.
- Opening a file whose size is not a multiple of `PAGE_SIZE` fails with `ErrorCode::Corruption`.

## BufferPool

- Each resident page occupies exactly one frame.
- A frame with a nonzero pin count is never evicted, so a pointer returned by `fetch_page`
  stays valid until the matching `unpin_page`.
- A dirty frame is written to disk before its frame is reused.
- Dirty pages are written only on eviction or `flush_page`; destroying the pool does no I/O.

## Record

- A `Value` has exactly one of the types NULL, INTEGER, FLOAT, or TEXT, and owns its data.
- Accessing a `Value` as the wrong type fails with `ErrorCode::TypeMismatch`.
- The on-disk representation of records is defined by `Serializer`, never by C++ object
  layout.

## Catalog

- A `Schema` has a non-empty table name and at least one column.
- Column names within a `Schema` are non-empty and unique.
- Table names within a `Catalog` are unique.
- The catalog exists only in memory.
