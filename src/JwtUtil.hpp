#pragma once

#include <string>

namespace utils {

struct JwtClaims {
  std::string userId;
};

// Creates JWT with `sub = userId`, exp = now + ttlSeconds
std::string signAccessToken(const std::string& userId,
                            const std::string& secret,
                            int ttlSeconds);

std::string signRefreshToken(const std::string& userId,
                             const std::string& secret,
                             int ttlSeconds);

JwtClaims verifyToken(const std::string& token, const std::string& secret);

}  // namespace utils
