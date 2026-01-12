#include "SQLiteUserRepository.hpp"

std::optional<User> SQLiteUserRepository::findByEmail(
    const std::string& email) {
  auto result = m_db->query<User>(
    "SELECT * FROM users WHERE email = ?", {email});
  if (result.empty()) return std::nullopt;
  return result.front();
}

void SQLiteUserRepository::create(const User& user) {
  m_db->execute(
    "INSERT INTO users(id, name, email, password_hash) VALUES(?,?,?,?)",
    {user.id, user.name, user.email, user.passwordHash});
}
