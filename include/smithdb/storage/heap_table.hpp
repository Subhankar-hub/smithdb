#pragma once

#include <optional>

#include "smithdb/common/types.hpp"
#include "smithdb/record/tuple.hpp"

namespace smithdb {

// Unordered collection of records stored in table pages, addressed by RID.
//
// PLACEHOLDER: this class defines the planned interface only. Every operation currently throws
// DatabaseError(NotImplemented). It will be implemented on top of BufferPool and slotted pages
// in a later phase (see the README roadmap).
class HeapTable {
public:
    // Stores `tuple` and returns its location.
    RID insert(const Tuple& tuple);

    // Returns the tuple at `rid`, or nullopt if no live record exists there.
    [[nodiscard]] std::optional<Tuple> get(RID rid);

    // Deletes the record at `rid`. Returns false if no live record exists there.
    bool remove(RID rid);
};

}  // namespace smithdb
