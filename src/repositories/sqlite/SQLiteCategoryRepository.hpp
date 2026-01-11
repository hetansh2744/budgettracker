#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "repositories/ICategoryRepository.hpp"
#include "repositories/sqlite/SQLiteDb.hpp"

namespace repo::sqlite {

class SQLiteCategoryRepository : public repo::ICategoryRepository {
 public:
  explicit SQLiteCategoryRepository(const std::shared_ptr<SQLiteDb>& db);

  std::vector<repo::Category> listForUser(const std::string& userId,
                                         const std::string& type) override;

  std::optional<repo::Category> findByIdForUser(const std::string& userId,
                                               const std::string& categoryId) override;

  repo::Category create(const std::string& userId,
                        const std::string& name,
                        const std::string& type) override;

  repo::Category updateName(const std::string& userId,
                            const std::string& categoryId,
                            const std::string& newName) override;

  void remove(const std::string& userId, const std::string& categoryId) override;

 private:
  std::shared_ptr<SQLiteDb> db_;
};

}  // namespace repo::sqlite
