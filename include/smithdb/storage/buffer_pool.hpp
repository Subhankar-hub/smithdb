#pragma once

#include <cstddef>
#include <optional>
#include <unordered_map>
#include <vector>

#include "smithdb/common/types.hpp"
#include "smithdb/storage/disk_manager.hpp"
#include "smithdb/storage/page.hpp"

namespace smithdb {

// Caches a fixed number of pages in memory on top of a DiskManager.
//
// Callers fetch a page (pinning it), use the returned pointer, and unpin it when done. A pinned
// page is never evicted, so the pointer stays valid until the matching unpin.
//
// Eviction is deliberately simple: when no frame is free, the first unpinned frame is reused,
// writing it back first if it is dirty. There is no LRU policy and no thread safety.
//
// Dirty pages are written back only on eviction or flush_page(); the destructor does no I/O.
//
// Invariants:
//   - Each resident page occupies exactly one frame.
//   - A frame with pin_count > 0 is never evicted.
//   - A dirty frame is written to disk before its frame is reused.
class BufferPool {
public:
    // Throws DatabaseError(InvalidArgument) if pool_size is 0.
    BufferPool(std::size_t pool_size, DiskManager& disk_manager);

    BufferPool(const BufferPool&) = delete;
    BufferPool& operator=(const BufferPool&) = delete;

    // Returns the pinned in-memory page for `page_id`, reading it from disk if necessary.
    // Returns nullptr if every frame is pinned. Throws DatabaseError if the page cannot be read.
    // The returned pointer is non-owning and valid until the matching unpin_page().
    [[nodiscard]] Page* fetch_page(PageId page_id);

    // Releases one pin on `page_id`; `dirty` marks the page as modified.
    // Throws DatabaseError(NotFound) if the page is not resident and
    // DatabaseError(InvalidArgument) if it is not pinned.
    void unpin_page(PageId page_id, bool dirty);

    // Writes the page to disk if it is resident and dirty. Does nothing for non-resident pages.
    void flush_page(PageId page_id);

    [[nodiscard]] std::size_t pool_size() const noexcept { return frames_.size(); }

private:
    struct Frame {
        Page page;
        std::optional<PageId> page_id;
        std::size_t pin_count{0};
        bool dirty{false};
    };

    // Returns the index of a frame that can hold a new page, or nullopt if all are pinned.
    std::optional<std::size_t> find_victim_frame();

    DiskManager& disk_manager_;
    std::vector<Frame> frames_;
    std::unordered_map<PageId, std::size_t> page_table_;
};

}  // namespace smithdb
