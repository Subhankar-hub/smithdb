#include "smithdb/storage/heap_table.hpp"

#include "smithdb/common/status.hpp"

namespace smithdb {

RID HeapTable::insert(const Tuple& /*tuple*/) {
    throw DatabaseError(ErrorCode::NotImplemented, "HeapTable::insert is not implemented");
}

std::optional<Tuple> HeapTable::get(RID /*rid*/) {
    throw DatabaseError(ErrorCode::NotImplemented, "HeapTable::get is not implemented");
}

bool HeapTable::remove(RID /*rid*/) {
    throw DatabaseError(ErrorCode::NotImplemented, "HeapTable::remove is not implemented");
}

}  // namespace smithdb
