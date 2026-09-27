#include <gtest/gtest.h>

#include <vector>

#include "smithdb/record/tuple.hpp"

namespace smithdb {
namespace {

TEST(TupleTest, DefaultIsEmpty) {
    const Tuple t;
    EXPECT_EQ(t.size(), 0U);
    EXPECT_TRUE(t.values().empty());
}

TEST(TupleTest, PreservesValuesInOrder) {
    const Tuple t({Value::integer(1), Value::null(), Value::floating(2.5), Value::text("x")});

    ASSERT_EQ(t.size(), 4U);
    EXPECT_EQ(t.values()[0].as_integer(), 1);
    EXPECT_TRUE(t.values()[1].is_null());
    EXPECT_DOUBLE_EQ(t.values()[2].as_float(), 2.5);
    EXPECT_EQ(t.values()[3].as_text(), "x");
}

TEST(TupleTest, EqualityComparesValues) {
    EXPECT_EQ(Tuple({Value::integer(1)}), Tuple({Value::integer(1)}));
    EXPECT_NE(Tuple({Value::integer(1)}), Tuple({Value::integer(2)}));
    EXPECT_NE(Tuple({Value::integer(1)}), Tuple({Value::integer(1), Value::null()}));
}

}  // namespace
}  // namespace smithdb
