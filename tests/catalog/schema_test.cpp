#include <gtest/gtest.h>

#include <vector>

#include "smithdb/catalog/schema.hpp"
#include "smithdb/common/status.hpp"

namespace smithdb {
namespace {

TEST(SchemaTest, StoresTableNameAndColumnsInOrder) {
    const Schema schema("users", {{"id", Type::Integer}, {"name", Type::Text}, {"score", Type::Float}});

    EXPECT_EQ(schema.table_name(), "users");
    const std::vector<Column> expected{
        {"id", Type::Integer}, {"name", Type::Text}, {"score", Type::Float}};
    EXPECT_EQ(schema.columns(), expected);
}

TEST(SchemaTest, RejectsDuplicateColumnNames) {
    try {
        Schema("users", {{"id", Type::Integer}, {"id", Type::Text}});
        FAIL() << "expected DatabaseError";
    } catch (const DatabaseError& e) {
        EXPECT_EQ(e.code(), ErrorCode::InvalidArgument);
    }
}

TEST(SchemaTest, RejectsEmptyNames) {
    EXPECT_THROW(Schema("", {{"id", Type::Integer}}), DatabaseError);
    EXPECT_THROW(Schema("users", {{"", Type::Integer}}), DatabaseError);
}

TEST(SchemaTest, RejectsTableWithoutColumns) {
    EXPECT_THROW(Schema("users", {}), DatabaseError);
}

}  // namespace
}  // namespace smithdb
