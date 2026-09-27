#include "smithdb/record/tuple.hpp"

#include <utility>

namespace smithdb {

Tuple::Tuple(std::vector<Value> values) : values_(std::move(values)) {}

}  // namespace smithdb
