#include "PasswordHasher.hpp"
#include <bcrypt/BCrypt.hpp>

std::string PasswordHasher::hash(const std::string& password) {
  return BCrypt::generateHash(password);
}

bool PasswordHasher::verify(const std::string& password,
                            const std::string& hash) {
  return BCrypt::validatePassword(password, hash);
}
