#include <gtest/gtest.h>

#include "smithdb/common/types.hpp"

namespace smithdb {
namespace {

TEST(RIDTest, EqualWhenPageAndSlotMatch) {
    EXPECT_EQ((RID{3, 7}), (RID{3, 7}));
}

TEST(RIDTest, DifferentPageOrSlotAreNotEqual) {
    EXPECT_NE((RID{3, 7}), (RID{4, 7}));
    EXPECT_NE((RID{3, 7}), (RID{3, 8}));
}

}  // namespace
}  // namespace smithdb
