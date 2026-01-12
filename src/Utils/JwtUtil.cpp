#include "JwtUtil.hpp"
#include <oatpp-jwt/jwt.hpp>

std::string JwtUtil::generateToken(const std::string& userId) {
  oatpp::jwt::Token token;
  token.setClaim("sub", userId);
  token.setIssuedAt(std::chrono::system_clock::now());
  token.setExpiresAt(std::chrono::system_clock::now() + std::chrono::hours(1));

  return oatpp::jwt::Jwt::sign(token, m_secret);
}

std::string JwtUtil::verifyAndGetUserId(const std::string& token) {
  auto decoded = oatpp::jwt::Jwt::verify(token, m_secret);
  return decoded.getClaim("sub").toString();
}
