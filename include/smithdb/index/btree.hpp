#pragma once

#include <optional>
#include <string>

#include "smithdb/common/status.hpp"
#include "smithdb/common/types.hpp"

namespace smithdb {

// Maps keys to record locations (RIDs).
//
// PLACEHOLDER: this class defines the planned interface of a disk-based B+ tree index only.
// Every operation currently throws DatabaseError(NotImplemented). The tree will be implemented
// in a later phase (see the README roadmap).
template <typename Key>
class BTree {
public:
    // Associates `key` with `rid`.
    void insert(const Key& /*key*/, RID /*rid*/) { not_implemented("insert"); }

    // Returns the RID associated with `key`, or nullopt if the key is absent.
    [[nodiscard]] std::optional<RID> search(const Key& /*key*/) const {
        not_implemented("search");
    }

    // Removes `key`. Returns false if the key is absent.
    bool remove(const Key& /*key*/) { not_implemented("remove"); }

private:
    [[noreturn]] static void not_implemented(const char* operation) {
        throw DatabaseError(ErrorCode::NotImplemented,
                            std::string("BTree::") + operation + " is not implemented");
    }
};

}  // namespace smithdb
