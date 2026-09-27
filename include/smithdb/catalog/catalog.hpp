#pragma once

#include <functional>
#include <map>
#include <string>
#include <string_view>

#include "smithdb/catalog/schema.hpp"

namespace smithdb {

// Registry of table schemas, keyed by table name.
//
// The catalog knows about schemas and tables only, not about file I/O. It is currently held
// in memory and is lost when the process exits; catalog persistence is planned for a later phase.
//
// Invariant: table names are unique.
class Catalog {
public:
    // Registers `schema`. Throws DatabaseError(AlreadyExists) if the table name is taken.
    void create_table(Schema schema);

    // Returns the schema for `name`, or nullptr if no such table exists. The pointer is
    // non-owning and remains valid for the lifetime of the Catalog.
    [[nodiscard]] const Schema* get_table(std::string_view name) const;

    [[nodiscard]] bool table_exists(std::string_view name) const;

private:
    std::map<std::string, Schema, std::less<>> tables_;
};

}  // namespace smithdb
