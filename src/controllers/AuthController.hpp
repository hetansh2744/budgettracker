#pragma once

#include <memory>

#include "oatpp/web/server/api/ApiController.hpp"
#include "dto/AuthDtos.hpp"
#include "services/AuthService.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)

class AuthController : public oatpp::web::server::api::ApiController {
 public:
  AuthController(OATPP_COMPONENT(std::shared_ptr<oatpp::web::mime::ContentMappers>, apiContentMappers),
                 const std::shared_ptr<services::AuthService>& authService)
      : oatpp::web::server::api::ApiController(apiContentMappers),
        authService_(authService) {}

  ENDPOINT("POST", "/auth/register", registerUser,
           BODY_DTO(Object<RegisterRequestDto>, body));

  ENDPOINT("POST", "/auth/login", loginUser,
           BODY_DTO(Object<LoginRequestDto>, body));

 private:
  std::shared_ptr<services::AuthService> authService_;
};

#include OATPP_CODEGEN_END(ApiController)
