#pragma once

#include <cstddef>

namespace smithdb {

// Tunable settings for a SmithDB instance.
struct Config {
    // Number of page frames held in memory by the buffer pool.
    std::size_t buffer_pool_size{64};
};

}  // namespace smithdb
