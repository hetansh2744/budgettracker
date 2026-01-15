#pragma once

#include "oatpp/web/server/api/ApiController.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)

class HealthController : public oatpp::web::server::api::ApiController {
 public:
  HealthController(OATPP_COMPONENT(std::shared_ptr<oatpp::web::mime::ContentMappers>, apiContentMappers))
      : oatpp::web::server::api::ApiController(apiContentMappers) {}

  ENDPOINT("GET", "/health", health) {
    return createDtoResponse(Status::CODE_200, oatpp::String("{\"status\":\"ok\"}"));
  }
};

#include OATPP_CODEGEN_END(ApiController)
