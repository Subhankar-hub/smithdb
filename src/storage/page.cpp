#include "smithdb/storage/page.hpp"

namespace smithdb {

static_assert(sizeof(Page) == PAGE_SIZE, "Page must contain exactly PAGE_SIZE bytes");

void Page::reset() noexcept { data_.fill(std::byte{0}); }

}  // namespace smithdb
