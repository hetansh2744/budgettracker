#pragma once

#include <memory>
#include <string>

#include "dto/TransactionDtos.hpp"
#include "repositories/ICategoryRepository.hpp"
#include "repositories/ITransactionRepository.hpp"

namespace services {

class TransactionService {
 public:
  TransactionService(const std::shared_ptr<repo::ITransactionRepository>& txRepo,
                     const std::shared_ptr<repo::ICategoryRepository>& categoryRepo);

  oatpp::Object<TransactionResponseDto> create(const std::string& userId,
                                              const oatpp::Object<TransactionCreateDto>& dto);

  oatpp::Object<TransactionResponseDto> getOne(const std::string& userId,
                                              const std::string& txId);

  oatpp::Object<TransactionResponseDto> update(const std::string& userId,
                                              const std::string& txId,
                                              const oatpp::Object<TransactionUpdateDto>& dto);

  void remove(const std::string& userId, const std::string& txId);

  oatpp::Object<PagedTransactionsDto> list(const std::string& userId,
                                          const repo::TransactionFilters& filters);

 private:
  std::shared_ptr<repo::ITransactionRepository> txRepo_;
  std::shared_ptr<repo::ICategoryRepository> categoryRepo_;
};

}  // namespace services
