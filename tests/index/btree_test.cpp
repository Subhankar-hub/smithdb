#include <gtest/gtest.h>

#include <cstdint>

#include "smithdb/common/status.hpp"
#include "smithdb/index/btree.hpp"

namespace smithdb {
namespace {

// BTree is a documented placeholder; these tests pin down that it reports NotImplemented
// rather than silently returning empty results.
TEST(BTreeTest, OperationsReportNotImplemented) {
    BTree<std::int64_t> tree;
    const auto expect_not_implemented = [](auto&& operation) {
        try {
            operation();
            FAIL() << "expected DatabaseError";
        } catch (const DatabaseError& e) {
            EXPECT_EQ(e.code(), ErrorCode::NotImplemented);
        }
    };

    expect_not_implemented([&] { tree.insert(1, RID{0, 0}); });
    expect_not_implemented([&] { (void)tree.search(1); });
    expect_not_implemented([&] { tree.remove(1); });
}

}  // namespace
}  // namespace smithdb
