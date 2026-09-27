#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

namespace smithdb {

// Logical SQL types supported by SmithDB.
enum class Type {
    Null,
    Integer,
    Float,
    Text,
};

// A single database value: NULL, a 64-bit integer, a 64-bit float, or text.
//
// A Value owns its data (TEXT values own their std::string), so it can be copied and moved
// freely. It is a logical, in-memory representation only; its on-disk encoding is defined by
// Serializer, not by this class's memory layout.
class Value {
public:
    // Constructs a NULL value.
    Value() = default;

    [[nodiscard]] static Value null() { return Value{}; }
    [[nodiscard]] static Value integer(std::int64_t v) { return Value{Storage{v}}; }
    [[nodiscard]] static Value floating(double v) { return Value{Storage{v}}; }
    [[nodiscard]] static Value text(std::string v) { return Value{Storage{std::move(v)}}; }

    [[nodiscard]] Type type() const noexcept;
    [[nodiscard]] bool is_null() const noexcept { return type() == Type::Null; }

    // Typed accessors. Each throws DatabaseError(TypeMismatch) if the value has another type.
    [[nodiscard]] std::int64_t as_integer() const;
    [[nodiscard]] double as_float() const;
    [[nodiscard]] std::string_view as_text() const;

    friend bool operator==(const Value&, const Value&) = default;

private:
    // Alternative order must match the Type enumerators.
    using Storage = std::variant<std::monostate, std::int64_t, double, std::string>;

    explicit Value(Storage storage) : storage_(std::move(storage)) {}

    Storage storage_;
};

}  // namespace smithdb
