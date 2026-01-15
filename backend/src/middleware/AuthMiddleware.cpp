#include "AuthMiddleware.hpp"
#include <oatpp/web/server/HttpRequest.hpp>

void AuthMiddleware::before(const oatpp::web::server::HttpRequest& request) {
  auto header = request.getHeader("Authorization");
  if (!header || header->find("Bearer ") != 0) {
    throw oatpp::web::protocol::http::HttpError(
      Status::CODE_401, "Missing auth token");
  }

  auto token = header->substr(7);
  auto userId = m_jwtUtil.verifyAndGetUserId(token);
  request.putAttribute("userId", userId);
}
