#include "smithdb/record/value.hpp"

#include <string>

#include "smithdb/common/status.hpp"

namespace smithdb {

namespace {

template <typename T, typename Variant>
const T& get_or_throw(const Variant& storage, const char* expected) {
    const T* value = std::get_if<T>(&storage);
    if (value == nullptr) {
        throw DatabaseError(ErrorCode::TypeMismatch,
                            std::string("value is not of type ") + expected);
    }
    return *value;
}

}  // namespace

Type Value::type() const noexcept { return static_cast<Type>(storage_.index()); }

std::int64_t Value::as_integer() const {
    return get_or_throw<std::int64_t>(storage_, "INTEGER");
}

double Value::as_float() const { return get_or_throw<double>(storage_, "FLOAT"); }

std::string_view Value::as_text() const { return get_or_throw<std::string>(storage_, "TEXT"); }

}  // namespace smithdb
