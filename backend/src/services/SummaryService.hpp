#pragma once

#include <memory>
#include <string>

#include "dto/SummaryDtos.hpp"
#include "repositories/ICategoryRepository.hpp"
#include "repositories/ITransactionRepository.hpp"

namespace services {

class SummaryService {
 public:
  SummaryService(const std::shared_ptr<repo::ITransactionRepository>& txRepo,
                 const std::shared_ptr<repo::ICategoryRepository>& categoryRepo);

  oatpp::Object<MonthlySummaryDto> monthly(const std::string& userId, int year);

  oatpp::Object<CategoryBreakdownDto> byCategory(const std::string& userId,
                                                const std::string& type,
                                                const std::string& startDate,
                                                const std::string& endDate);

  oatpp::Object<CashflowSummaryDto> cashflow(const std::string& userId,
                                             const std::string& startDate,
                                             const std::string& endDate);

 private:
  std::shared_ptr<repo::ITransactionRepository> txRepo_;
  std::shared_ptr<repo::ICategoryRepository> categoryRepo_;
};

}  // namespace services
