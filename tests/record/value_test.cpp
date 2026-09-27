#include <gtest/gtest.h>

#include <string>

#include "smithdb/common/status.hpp"
#include "smithdb/record/value.hpp"

namespace smithdb {
namespace {

TEST(ValueTest, DefaultIsNull) {
    const Value v;
    EXPECT_EQ(v.type(), Type::Null);
    EXPECT_TRUE(v.is_null());
    EXPECT_EQ(v, Value::null());
}

TEST(ValueTest, Integer) {
    const Value v = Value::integer(-42);
    EXPECT_EQ(v.type(), Type::Integer);
    EXPECT_FALSE(v.is_null());
    EXPECT_EQ(v.as_integer(), -42);
}

TEST(ValueTest, Float) {
    const Value v = Value::floating(3.5);
    EXPECT_EQ(v.type(), Type::Float);
    EXPECT_DOUBLE_EQ(v.as_float(), 3.5);
}

TEST(ValueTest, Text) {
    const Value v = Value::text("hello");
    EXPECT_EQ(v.type(), Type::Text);
    EXPECT_EQ(v.as_text(), "hello");
}

TEST(ValueTest, TextCopyOwnsItsData) {
    Value original = Value::text("abc");
    const Value copy = original;
    original = Value::integer(1);
    EXPECT_EQ(copy.as_text(), "abc");
}

TEST(ValueTest, EqualityComparesTypeAndContent) {
    EXPECT_EQ(Value::integer(1), Value::integer(1));
    EXPECT_NE(Value::integer(1), Value::integer(2));
    EXPECT_NE(Value::integer(1), Value::floating(1.0));
    EXPECT_NE(Value::text(""), Value::null());
}

TEST(ValueTest, WrongTypeAccessThrowsTypeMismatch) {
    const Value v = Value::integer(7);
    try {
        (void)v.as_text();
        FAIL() << "expected DatabaseError";
    } catch (const DatabaseError& e) {
        EXPECT_EQ(e.code(), ErrorCode::TypeMismatch);
    }
    EXPECT_THROW((void)Value::null().as_integer(), DatabaseError);
    EXPECT_THROW((void)Value::text("x").as_float(), DatabaseError);
}

}  // namespace
}  // namespace smithdb
