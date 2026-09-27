#include "smithdb/record/serializer.hpp"

#include "smithdb/common/status.hpp"

namespace smithdb {

std::vector<std::byte> Serializer::serialize(const Tuple& /*tuple*/) {
    throw DatabaseError(ErrorCode::NotImplemented, "Serializer::serialize is not implemented");
}

Tuple Serializer::deserialize(std::span<const std::byte> /*bytes*/) {
    throw DatabaseError(ErrorCode::NotImplemented, "Serializer::deserialize is not implemented");
}

}  // namespace smithdb
