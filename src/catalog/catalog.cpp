#include "smithdb/catalog/catalog.hpp"

#include <utility>

#include "smithdb/common/status.hpp"

namespace smithdb {

void Catalog::create_table(Schema schema) {
    std::string name = schema.table_name();
    if (tables_.contains(name)) {
        throw DatabaseError(ErrorCode::AlreadyExists, "table '" + name + "' already exists");
    }
    tables_.emplace(std::move(name), std::move(schema));
}

const Schema* Catalog::get_table(std::string_view name) const {
    const auto it = tables_.find(name);
    return it == tables_.end() ? nullptr : &it->second;
}

bool Catalog::table_exists(std::string_view name) const { return tables_.find(name) != tables_.end(); }

}  // namespace smithdb
