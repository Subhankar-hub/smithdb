#pragma once

#include <cstddef>

namespace smithdb {

// Size in bytes of every page, both in memory and on disk. This is the single source of truth
// for the page size; changing it changes the physical file format.
inline constexpr std::size_t PAGE_SIZE = 4096;

}  // namespace smithdb
