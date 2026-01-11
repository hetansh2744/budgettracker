#pragma once

#include <memory>
#include <string>

#include "oatpp/web/server/interceptor/RequestInterceptor.hpp"
#include "oatpp/web/protocol/http/incoming/Request.hpp"

namespace middleware {

// Stores userId into request attribute: "userId"
class AuthMiddleware : public oatpp::web::server::interceptor::RequestInterceptor {
 public:
  AuthMiddleware(const std::string& jwtAccessSecret);

  std::shared_ptr<oatpp::web::protocol::http::incoming::Request> intercept(
      const std::shared_ptr<oatpp::web::protocol::http::incoming::Request>& request) override;

 private:
  std::string jwtAccessSecret_;
};

}  // namespace middleware
