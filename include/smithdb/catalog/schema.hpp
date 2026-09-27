#pragma once

#include <string>
#include <vector>

#include "smithdb/catalog/column.hpp"

namespace smithdb {

// The logical definition of a table: its name and ordered columns.
//
// Invariants (enforced by the constructor):
//   - The table name is not empty.
//   - There is at least one column.
//   - Column names are not empty and are unique within the table.
class Schema {
public:
    // Throws DatabaseError(InvalidArgument) if any invariant is violated.
    Schema(std::string table_name, std::vector<Column> columns);

    [[nodiscard]] const std::string& table_name() const noexcept { return table_name_; }
    [[nodiscard]] const std::vector<Column>& columns() const noexcept { return columns_; }

private:
    std::string table_name_;
    std::vector<Column> columns_;
};

}  // namespace smithdb
