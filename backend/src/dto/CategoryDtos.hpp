#pragma once

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/Types.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class CategoryDto : public oatpp::DTO {
  DTO_INIT(CategoryDto, DTO)
  DTO_FIELD(String, id);
  DTO_FIELD(String, name);
  DTO_FIELD(String, type); // "INCOME" | "EXPENSE"
  DTO_FIELD(String, createdAt);
};

class CategoryCreateDto : public oatpp::DTO {
  DTO_INIT(CategoryCreateDto, DTO)
  DTO_FIELD(String, name);
  DTO_FIELD(String, type); // "INCOME" | "EXPENSE"
};

class CategoryUpdateDto : public oatpp::DTO {
  DTO_INIT(CategoryUpdateDto, DTO)
  DTO_FIELD(String, name);
};

#include OATPP_CODEGEN_END(DTO)
