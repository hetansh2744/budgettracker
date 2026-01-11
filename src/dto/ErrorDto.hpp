#pragma once

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/Types.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class ErrorInnerDto : public oatpp::DTO {
  DTO_INIT(ErrorInnerDto, DTO)
  DTO_FIELD(String, code);
  DTO_FIELD(String, message);
  DTO_FIELD(Any, details);
};

class ErrorDto : public oatpp::DTO {
  DTO_INIT(ErrorDto, DTO)
  DTO_FIELD(Object<ErrorInnerDto>, error);
};

#include OATPP_CODEGEN_END(DTO)
