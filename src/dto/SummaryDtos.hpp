#pragma once

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/Types.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class MonthlySummaryItemDto : public oatpp::DTO {
  DTO_INIT(MonthlySummaryItemDto, DTO)
  DTO_FIELD(Int32, month); // 1..12
  DTO_FIELD(Int64, incomeCents);
  DTO_FIELD(Int64, expenseCents);
  DTO_FIELD(Int64, netCents); // income - expense
};

class MonthlySummaryDto : public oatpp::DTO {
  DTO_INIT(MonthlySummaryDto, DTO)
  DTO_FIELD(Int32, year);
  DTO_FIELD(List<Object<MonthlySummaryItemDto>>, months);
};

class CategoryBreakdownItemDto : public oatpp::DTO {
  DTO_INIT(CategoryBreakdownItemDto, DTO)
  DTO_FIELD(String, categoryId);
  DTO_FIELD(String, categoryName);
  DTO_FIELD(Int64, totalCents);
};

class CategoryBreakdownDto : public oatpp::DTO {
  DTO_INIT(CategoryBreakdownDto, DTO)
  DTO_FIELD(String, type); // "INCOME"|"EXPENSE"
  DTO_FIELD(String, startDate);
  DTO_FIELD(String, endDate);
  DTO_FIELD(List<Object<CategoryBreakdownItemDto>>, items);
};

class CashflowSummaryDto : public oatpp::DTO {
  DTO_INIT(CashflowSummaryDto, DTO)
  DTO_FIELD(String, startDate);
  DTO_FIELD(String, endDate);
  DTO_FIELD(Int64, incomeCents);
  DTO_FIELD(Int64, expenseCents);
  DTO_FIELD(Int64, netCents);
};

#include OATPP_CODEGEN_END(DTO)
