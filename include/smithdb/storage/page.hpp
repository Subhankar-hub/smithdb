#pragma once

#include <array>
#include <cstddef>

#include "smithdb/common/constants.hpp"

namespace smithdb {

// A fixed-size, in-memory copy of one database page.
//
// A Page is a raw byte container. It has no knowledge of tables, records, or file offsets;
// higher layers define how its bytes are interpreted.
//
// Invariant: a Page always owns exactly PAGE_SIZE bytes.
class Page {
public:
    // Constructs a zero-filled page.
    Page() = default;

    [[nodiscard]] std::byte* data() noexcept { return data_.data(); }
    [[nodiscard]] const std::byte* data() const noexcept { return data_.data(); }

    // Sets every byte of the page to zero.
    void reset() noexcept;

private:
    std::array<std::byte, PAGE_SIZE> data_{};
};

}  // namespace smithdb
