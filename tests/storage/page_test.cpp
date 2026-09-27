#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <span>

#include "smithdb/common/constants.hpp"
#include "smithdb/storage/page.hpp"

namespace smithdb {
namespace {

bool all_zero(const Page& page) {
    const std::span<const std::byte> bytes(page.data(), PAGE_SIZE);
    return std::ranges::all_of(bytes, [](std::byte b) { return b == std::byte{0}; });
}

TEST(PageTest, HoldsExactlyPageSizeBytes) {
    EXPECT_EQ(sizeof(Page), PAGE_SIZE);
}

TEST(PageTest, NewPageIsZeroFilled) {
    const Page page;
    EXPECT_TRUE(all_zero(page));
}

TEST(PageTest, MutableAccessIsVisibleThroughConstAccess) {
    Page page;
    page.data()[0] = std::byte{0xAB};
    page.data()[PAGE_SIZE - 1] = std::byte{0xCD};

    const Page& view = page;
    EXPECT_EQ(view.data()[0], std::byte{0xAB});
    EXPECT_EQ(view.data()[PAGE_SIZE - 1], std::byte{0xCD});
    EXPECT_EQ(view.data(), page.data());
}

TEST(PageTest, ResetZeroesAllBytes) {
    Page page;
    std::ranges::fill(std::span<std::byte>(page.data(), PAGE_SIZE), std::byte{0xFF});
    ASSERT_FALSE(all_zero(page));

    page.reset();
    EXPECT_TRUE(all_zero(page));
}

}  // namespace
}  // namespace smithdb
