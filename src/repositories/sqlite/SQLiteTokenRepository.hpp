#pragma once

#include <memory>
#include <optional>
#include <string>

#include "repositories/ITokenRepository.hpp"
#include "repositories/sqlite/SQLiteDb.hpp"

namespace repo::sqlite {

class SQLiteTokenRepository : public repo::ITokenRepository {
 public:
  explicit SQLiteTokenRepository(const std::shared_ptr<SQLiteDb>& db);

  repo::RefreshToken create(const std::string& userId, const std::string& tokenHash) override;

  std::optional<repo::RefreshToken> findByIdForUser(const std::string& userId,
                                                    const std::string& tokenId) override;

  void revoke(const std::string& userId, const std::string& tokenId) override;

 private:
  std::shared_ptr<SQLiteDb> db_;
};

}  // namespace repo::sqlite
