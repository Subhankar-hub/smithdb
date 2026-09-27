#include <gtest/gtest.h>

#include "smithdb/catalog/catalog.hpp"
#include "smithdb/common/status.hpp"

namespace smithdb {
namespace {

TEST(CatalogTest, CreatedTableCanBeLookedUp) {
    Catalog catalog;
    catalog.create_table(Schema("users", {{"id", Type::Integer}, {"name", Type::Text}}));

    EXPECT_TRUE(catalog.table_exists("users"));
    const Schema* schema = catalog.get_table("users");
    ASSERT_NE(schema, nullptr);
    EXPECT_EQ(schema->table_name(), "users");
    EXPECT_EQ(schema->columns().size(), 2U);
}

TEST(CatalogTest, UnknownTableIsAbsent) {
    Catalog catalog;
    catalog.create_table(Schema("users", {{"id", Type::Integer}}));

    EXPECT_FALSE(catalog.table_exists("orders"));
    EXPECT_EQ(catalog.get_table("orders"), nullptr);
}

TEST(CatalogTest, RejectsDuplicateTableName) {
    Catalog catalog;
    catalog.create_table(Schema("users", {{"id", Type::Integer}}));
    try {
        catalog.create_table(Schema("users", {{"other", Type::Text}}));
        FAIL() << "expected DatabaseError";
    } catch (const DatabaseError& e) {
        EXPECT_EQ(e.code(), ErrorCode::AlreadyExists);
    }
    EXPECT_EQ(catalog.get_table("users")->columns().front().name, "id");
}

}  // namespace
}  // namespace smithdb
