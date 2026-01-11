#pragma once

#include <optional>
#include <string>
#include <vector>

#include "repositories/Models.hpp"

namespace repo {

struct TransactionFilters {
  std::string type;       // optional empty
  std::string startDate;  // optional empty
  std::string endDate;    // optional empty
  std::string categoryId; // optional empty
  int64_t minAmountCents = -1;
  int64_t maxAmountCents = -1;

  int page = 1;
  int pageSize = 20;

  std::string sort; // date_desc, date_asc, amount_desc, amount_asc
};

struct PagedTransactions {
  std::vector<Transaction> items;
  int page = 1;
  int pageSize = 20;
  int totalItems = 0;
  int totalPages = 0;
};

class ITransactionRepository {
 public:
  virtual ~ITransactionRepository() = default;

  virtual Transaction create(const Transaction& tx) = 0;

  virtual std::optional<Transaction> findByIdForUser(const std::string& userId,
                                                     const std::string& txId) = 0;

  virtual Transaction update(const std::string& userId, const Transaction& tx) = 0;

  virtual void remove(const std::string& userId, const std::string& txId) = 0;

  virtual PagedTransactions listForUser(const std::string& userId,
                                        const TransactionFilters& filters) = 0;

  // Summary helpers
  virtual int64_t sumByTypeForUserInDateRange(const std::string& userId,
                                             const std::string& type,
                                             const std::string& startDate,
                                             const std::string& endDate) = 0;

  virtual std::vector<std::pair<std::string, int64_t>> sumByCategoryForUserInDateRange(
      const std::string& userId,
      const std::string& type,
      const std::string& startDate,
      const std::string& endDate) = 0;

  // monthly: returns size 12 (month 1..12)
  virtual std::vector<std::pair<int, int64_t>> sumMonthlyForUserByType(
      const std::string& userId, const std::string& type, int year) = 0;
};

}  // namespace repo
