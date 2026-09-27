#pragma once

#include <cstddef>
#include <vector>

#include "smithdb/record/value.hpp"

namespace smithdb {

// One row: an ordered list of values. A Tuple owns its values.
class Tuple {
public:
    Tuple() = default;
    explicit Tuple(std::vector<Value> values);

    [[nodiscard]] const std::vector<Value>& values() const noexcept { return values_; }
    [[nodiscard]] std::size_t size() const noexcept { return values_.size(); }

    friend bool operator==(const Tuple&, const Tuple&) = default;

private:
    std::vector<Value> values_;
};

}  // namespace smithdb
