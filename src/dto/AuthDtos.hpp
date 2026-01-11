#pragma once

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/Types.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class RegisterRequestDto : public oatpp::DTO {
  DTO_INIT(RegisterRequestDto, DTO)
  DTO_FIELD(String, name);
  DTO_FIELD(String, email);
  DTO_FIELD(String, password);
};

class LoginRequestDto : public oatpp::DTO {
  DTO_INIT(LoginRequestDto, DTO)
  DTO_FIELD(String, email);
  DTO_FIELD(String, password);
};

class AuthResponseDto : public oatpp::DTO {
  DTO_INIT(AuthResponseDto, DTO)
  DTO_FIELD(String, accessToken);
  DTO_FIELD(String, refreshToken); // optional (may be null)
};

#include OATPP_CODEGEN_END(DTO)
