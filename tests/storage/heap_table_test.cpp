#include <gtest/gtest.h>

#include "smithdb/common/status.hpp"
#include "smithdb/storage/heap_table.hpp"

namespace smithdb {
namespace {

// HeapTable is a documented placeholder; these tests pin down that it reports NotImplemented
// rather than silently pretending to store records.
TEST(HeapTableTest, OperationsReportNotImplemented) {
    HeapTable table;
    const auto expect_not_implemented = [](auto&& operation) {
        try {
            operation();
            FAIL() << "expected DatabaseError";
        } catch (const DatabaseError& e) {
            EXPECT_EQ(e.code(), ErrorCode::NotImplemented);
        }
    };

    expect_not_implemented([&] { table.insert(Tuple{}); });
    expect_not_implemented([&] { (void)table.get(RID{0, 0}); });
    expect_not_implemented([&] { table.remove(RID{0, 0}); });
}

}  // namespace
}  // namespace smithdb
