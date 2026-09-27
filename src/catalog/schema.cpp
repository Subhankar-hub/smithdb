#include "smithdb/catalog/schema.hpp"

#include <string_view>
#include <unordered_set>
#include <utility>

#include "smithdb/common/status.hpp"

namespace smithdb {

Schema::Schema(std::string table_name, std::vector<Column> columns)
    : table_name_(std::move(table_name)), columns_(std::move(columns)) {
    if (table_name_.empty()) {
        throw DatabaseError(ErrorCode::InvalidArgument, "table name must not be empty");
    }
    if (columns_.empty()) {
        throw DatabaseError(ErrorCode::InvalidArgument,
                            "table '" + table_name_ + "' must have at least one column");
    }

    std::unordered_set<std::string_view> seen;
    for (const Column& column : columns_) {
        if (column.name.empty()) {
            throw DatabaseError(ErrorCode::InvalidArgument,
                                "table '" + table_name_ + "' has a column with an empty name");
        }
        if (!seen.insert(column.name).second) {
            throw DatabaseError(ErrorCode::InvalidArgument, "table '" + table_name_ +
                                                                "' has duplicate column '" +
                                                                column.name + "'");
        }
    }
}

}  // namespace smithdb
