#pragma once

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/Types.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class TransactionCreateDto : public oatpp::DTO {
  DTO_INIT(TransactionCreateDto, DTO)

  DTO_FIELD(String, type); // "INCOME" | "EXPENSE"
  DTO_FIELD(Int64, amountCents);
  DTO_FIELD(String, currency); // "CAD"
  DTO_FIELD(String, date); // "YYYY-MM-DD"
  DTO_FIELD(String, categoryId);
  DTO_FIELD(String, sourceOrMerchant); // optional
  DTO_FIELD(String, note); // optional
};

class TransactionUpdateDto : public oatpp::DTO {
  DTO_INIT(TransactionUpdateDto, DTO)

  DTO_FIELD(String, type); // optional for update
  DTO_FIELD(Int64, amountCents);
  DTO_FIELD(String, currency);
  DTO_FIELD(String, date);
  DTO_FIELD(String, categoryId);
  DTO_FIELD(String, sourceOrMerchant);
  DTO_FIELD(String, note);
};

class TransactionResponseDto : public oatpp::DTO {
  DTO_INIT(TransactionResponseDto, DTO)

  DTO_FIELD(String, id);
  DTO_FIELD(String, type);
  DTO_FIELD(Int64, amountCents);
  DTO_FIELD(String, currency);
  DTO_FIELD(String, date);

  DTO_FIELD(String, categoryId);
  DTO_FIELD(String, categoryName);

  DTO_FIELD(String, sourceOrMerchant);
  DTO_FIELD(String, note);

  DTO_FIELD(String, createdAt);
  DTO_FIELD(String, updatedAt);
};

class PagedTransactionsDto : public oatpp::DTO {
  DTO_INIT(PagedTransactionsDto, DTO)

  DTO_FIELD(List<Object<TransactionResponseDto>>, items);

  DTO_FIELD(Int32, page);
  DTO_FIELD(Int32, pageSize);
  DTO_FIELD(Int32, totalItems);
  DTO_FIELD(Int32, totalPages);
};

#include OATPP_CODEGEN_END(DTO)
