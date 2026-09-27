#pragma once

#include <cstddef>
#include <span>
#include <vector>

#include "smithdb/record/tuple.hpp"

namespace smithdb {

// Converts between the logical Tuple representation and its on-disk byte encoding.
//
// The database's binary representation is independent of C++ object layout. Tuples and Values
// are never copied to disk with memcpy or reinterpret_cast: their in-memory layout depends on the
// compiler, standard library, and platform, and TEXT values hold pointers to heap memory. The
// on-disk encoding is instead defined explicitly (field by field, with a fixed byte order) so a
// file written by one build can be read by another.
//
// PLACEHOLDER: the record encoding has not been designed yet. Both functions currently throw
// DatabaseError(NotImplemented). The format will be documented in docs/file_format.md once it
// exists.
class Serializer {
public:
    [[nodiscard]] static std::vector<std::byte> serialize(const Tuple& tuple);

    [[nodiscard]] static Tuple deserialize(std::span<const std::byte> bytes);
};

}  // namespace smithdb
