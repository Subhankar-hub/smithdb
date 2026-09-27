# SmithDB File Format

This document describes only what is implemented today. It will grow as page layouts and record
encodings are added.

## Physical layout

- A database is stored in a single file.
- The file is a sequence of fixed-size pages.
- The page size is **4096 bytes** (`PAGE_SIZE` in `common/constants.hpp`).
- Page IDs are unsigned **32-bit** integers (`PageId`).
- Page `N` occupies bytes `[N * 4096, (N + 1) * 4096)` of the file.
- The file size is always an exact multiple of the page size. A file whose size is not a
  multiple of 4096 is rejected as corrupt when opened.
- The number of pages is `file_size / 4096`.

```text
offset 0        4096      8192      12288
       +---------+---------+---------+----
       | page 0  | page 1  | page 2  | ...
       +---------+---------+---------+----
```

## Page allocation

- Page IDs are allocated sequentially, starting at 0.
- Allocating a page appends one zero-filled page to the end of the file.
- Pages are never freed or reused.

## Page contents

The contents of a page are currently uninterpreted bytes. There is no file header, page header,
checksum, or record layout yet.
