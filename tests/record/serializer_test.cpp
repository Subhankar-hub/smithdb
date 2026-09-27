#include <gtest/gtest.h>

#include <cstddef>
#include <vector>

#include "smithdb/common/status.hpp"
#include "smithdb/record/serializer.hpp"

namespace smithdb {
namespace {

// Serializer is a documented placeholder until the record format is designed; these tests pin
// down that it reports NotImplemented rather than producing an undocumented encoding.
TEST(SerializerTest, SerializeReportsNotImplemented) {
    try {
        (void)Serializer::serialize(Tuple({Value::integer(1)}));
        FAIL() << "expected DatabaseError";
    } catch (const DatabaseError& e) {
        EXPECT_EQ(e.code(), ErrorCode::NotImplemented);
    }
}

TEST(SerializerTest, DeserializeReportsNotImplemented) {
    const std::vector<std::byte> bytes(8, std::byte{0});
    try {
        (void)Serializer::deserialize(bytes);
        FAIL() << "expected DatabaseError";
    } catch (const DatabaseError& e) {
        EXPECT_EQ(e.code(), ErrorCode::NotImplemented);
    }
}

}  // namespace
}  // namespace smithdb
