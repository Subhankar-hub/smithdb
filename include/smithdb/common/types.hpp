#pragma once

#include <cstdint>

namespace smithdb {

// Identifies a physical page in the database file. Page N starts at byte offset N * PAGE_SIZE.
using PageId = std::uint32_t;

// Identifies a record slot within a page.
using SlotId = std::uint16_t;

// Record identifier: the physical location of a record as a (page, slot) pair.
struct RID {
    PageId page_id{0};
    SlotId slot_id{0};

    friend bool operator==(const RID&, const RID&) = default;
};

}  // namespace smithdb
