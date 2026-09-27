#pragma once

#include <filesystem>
#include <fstream>

#include "smithdb/common/types.hpp"
#include "smithdb/storage/page.hpp"

namespace smithdb {

// Owns the database file and performs page-granular I/O on it.
//
// The DiskManager knows about files and pages only; it has no knowledge of tables, records,
// or SQL. The database file is a sequence of PAGE_SIZE pages, where page N is stored at byte
// offset N * PAGE_SIZE.
//
// Invariants:
//   - The file size is always page_count() * PAGE_SIZE.
//   - Page IDs are allocated sequentially starting at 0 and are never reused.
//   - Reads and writes always transfer complete pages.
//
// All failures are reported by throwing DatabaseError.
class DiskManager {
public:
    // Opens the database file at `path`, creating an empty file if it does not exist.
    // Throws DatabaseError(Corruption) if the existing file size is not a multiple of PAGE_SIZE.
    explicit DiskManager(const std::filesystem::path& path);

    DiskManager(const DiskManager&) = delete;
    DiskManager& operator=(const DiskManager&) = delete;

    // Flushes buffered writes; errors during destruction are ignored.
    ~DiskManager();

    // Extends the file by one zero-filled page and returns its ID.
    PageId allocate_page();

    // Reads page `page_id` into `page`. Throws DatabaseError(OutOfRange) if the page does not exist.
    void read_page(PageId page_id, Page& page);

    // Writes `page` to page `page_id`. Throws DatabaseError(OutOfRange) if the page does not exist.
    void write_page(PageId page_id, const Page& page);

    // Pushes buffered writes to the operating system.
    void flush();

    // Number of pages currently in the database file.
    [[nodiscard]] PageId page_count() const noexcept { return page_count_; }

    [[nodiscard]] const std::filesystem::path& path() const noexcept { return path_; }

private:
    void check_page_exists(PageId page_id) const;
    void write_at(PageId page_id, const Page& page);

    std::filesystem::path path_;
    std::fstream file_;
    PageId page_count_{0};
};

}  // namespace smithdb
