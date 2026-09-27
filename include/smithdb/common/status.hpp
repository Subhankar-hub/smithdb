#pragma once

#include <stdexcept>
#include <string>

namespace smithdb {

// Category of a SmithDB failure.
enum class ErrorCode {
    IoError,          // The operating system reported a file I/O failure.
    Corruption,       // On-disk data violates a format invariant.
    InvalidArgument,  // The caller passed a value that violates a precondition.
    OutOfRange,       // An identifier refers to something that does not exist (e.g. a page).
    NotFound,         // A requested object is not present.
    AlreadyExists,    // An object with the same identity already exists.
    TypeMismatch,     // A value was accessed as the wrong type.
    NotImplemented,   // The operation is part of the planned interface but not implemented yet.
};

// Success is reported by returning normally; failure is reported by throwing DatabaseError.
class DatabaseError : public std::runtime_error {
public:
    DatabaseError(ErrorCode code, const std::string& message)
        : std::runtime_error(message), code_(code) {}

    [[nodiscard]] ErrorCode code() const noexcept { return code_; }

private:
    ErrorCode code_;
};

}  // namespace smithdb
