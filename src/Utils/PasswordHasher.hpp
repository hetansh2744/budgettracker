#pragma once

#include <string>

namespace utils {

// Wrapper around a password hashing library (bcrypt/argon2).
// We'll implement using bcrypt in .cpp (or a small dependency).
class PasswordHasher {
 public:
  std::string hash(const std::string& password) const;
  bool verify(const std::string& password, const std::string& passwordHash) const;
};

}  // namespace utils
