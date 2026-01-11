#pragma once

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "repositories/ITransactionRepository.hpp"
#include "repositories/sqlite/SQLiteDb.hpp"

namespace repo::sqlite {

class SQLiteTransactionRepository : public repo::ITransactionRepository {
 public:
  explicit SQLiteTransactionRepository(const std::shared_ptr<SQLiteDb>& db);

  repo::Transaction create(const repo::Transaction& tx) override;

  std::optional<repo::Transaction> findByIdForUser(const std::string& userId,
                                                   const std::string& txId) override;

  repo::Transaction update(const std::string& userId, const repo::Transaction& tx) override;

  void remove(const std::string& userId, const std::string& txId) override;

  repo::PagedTransactions listForUser(const std::string& userId,
                                      const repo::TransactionFilters& filters) override;

  int64_t sumByTypeForUserInDateRange(const std::string& userId,
                                     const std::string& type,
                                     const std::string& startDate,
                                     const std::string& endDate) override;

  std::vector<std::pair<std::string, int64_t>> sumByCategoryForUserInDateRange(
      const std::string& userId,
      const std::string& type,
      const std::string& startDate,
      const std::string& endDate) override;

  std::vector<std::pair<int, int64_t>> sumMonthlyForUserByType(
      const std::string& userId, const std::string& type, int year) override;

 private:
  std::shared_ptr<SQLiteDb> db_;
};

}  // namespace repo::sqlite
