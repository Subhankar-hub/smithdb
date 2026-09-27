#include "smithdb/storage/buffer_pool.hpp"

#include <string>

#include "smithdb/common/status.hpp"

namespace smithdb {

BufferPool::BufferPool(std::size_t pool_size, DiskManager& disk_manager)
    : disk_manager_(disk_manager) {
    if (pool_size == 0) {
        throw DatabaseError(ErrorCode::InvalidArgument, "buffer pool size must be positive");
    }
    frames_.resize(pool_size);
}

Page* BufferPool::fetch_page(PageId page_id) {
    if (const auto it = page_table_.find(page_id); it != page_table_.end()) {
        Frame& frame = frames_[it->second];
        ++frame.pin_count;
        return &frame.page;
    }

    const std::optional<std::size_t> victim = find_victim_frame();
    if (!victim) {
        return nullptr;
    }

    Frame& frame = frames_[*victim];
    if (frame.page_id) {
        if (frame.dirty) {
            disk_manager_.write_page(*frame.page_id, frame.page);
        }
        page_table_.erase(*frame.page_id);
        frame.page_id.reset();
        frame.dirty = false;
    }

    disk_manager_.read_page(page_id, frame.page);
    frame.page_id = page_id;
    frame.pin_count = 1;
    page_table_.emplace(page_id, *victim);
    return &frame.page;
}

void BufferPool::unpin_page(PageId page_id, bool dirty) {
    const auto it = page_table_.find(page_id);
    if (it == page_table_.end()) {
        throw DatabaseError(ErrorCode::NotFound,
                            "page " + std::to_string(page_id) + " is not in the buffer pool");
    }
    Frame& frame = frames_[it->second];
    if (frame.pin_count == 0) {
        throw DatabaseError(ErrorCode::InvalidArgument,
                            "page " + std::to_string(page_id) + " is not pinned");
    }
    --frame.pin_count;
    frame.dirty = frame.dirty || dirty;
}

void BufferPool::flush_page(PageId page_id) {
    const auto it = page_table_.find(page_id);
    if (it == page_table_.end()) {
        return;
    }
    Frame& frame = frames_[it->second];
    if (frame.dirty) {
        disk_manager_.write_page(page_id, frame.page);
        frame.dirty = false;
    }
}

std::optional<std::size_t> BufferPool::find_victim_frame() {
    std::optional<std::size_t> unpinned;
    for (std::size_t i = 0; i < frames_.size(); ++i) {
        if (!frames_[i].page_id) {
            return i;
        }
        if (!unpinned && frames_[i].pin_count == 0) {
            unpinned = i;
        }
    }
    return unpinned;
}

}  // namespace smithdb
