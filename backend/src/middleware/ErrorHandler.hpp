#pragma once

#include "oatpp/web/server/handler/ErrorHandler.hpp"
#include "oatpp/web/protocol/http/outgoing/Response.hpp"

namespace middleware {

// Converts exceptions (AppError etc.) into consistent ErrorDto responses
class ErrorHandler : public oatpp::web::server::handler::ErrorHandler {
 public:
  std::shared_ptr<oatpp::web::protocol::http::outgoing::Response> handleError(
      const oatpp::web::protocol::http::Status& status,
      const oatpp::String& message,
      const oatpp::web::protocol::http::incoming::Request* request) override;
};

}  // namespace middleware
