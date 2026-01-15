#pragma once

#include <memory>
#include <optional>
#include <string>

#include "repositories/IUserRepository.hpp"
#include "repositories/sqlite/SQLiteDb.hpp"

namespace repo::sqlite {

class SQLiteUserRepository : public repo::IUserRepository {
 public:
  explicit SQLiteUserRepository(const std::shared_ptr<SQLiteDb>& db);

  std::optional<repo::User> findByEmail(const std::string& email) override;
  std::optional<repo::User> findById(const std::string& userId) override;

  repo::User createUser(const std::string& name,
                        const std::string& email,
                        const std::string& passwordHash) override;

 private:
  std::shared_ptr<SQLiteDb> db_;
};

}  // namespace repo::sqlite
