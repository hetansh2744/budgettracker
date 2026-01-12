#include "TransactionService.hpp"

TransactionResponseDto::ObjectWrapper
TransactionService::create(const std::string& userId,
                           const TransactionCreateDto::ObjectWrapper& dto) {
  Transaction tx;
  tx.id = uuid();
  tx.userId = userId;
  tx.categoryId = dto->categoryId;
  tx.type = dto->type;
  tx.amountCents = dto->amount * 100;
  tx.currency = dto->currency;
  tx.date = dto->date;
  tx.note = dto->note;

  m_txRepo->create(tx);

  auto res = TransactionResponseDto::createShared();
  res->id = tx.id;
  res->amount = dto->amount;
  res->type = dto->type;
  res->currency = tx.currency;
  return res;
}
