#pragma once

#include <string>

#include "smithdb/record/value.hpp"

namespace smithdb {

// A named, typed column of a table.
struct Column {
    std::string name;
    Type type{Type::Null};

    friend bool operator==(const Column&, const Column&) = default;
};

}  // namespace smithdb
