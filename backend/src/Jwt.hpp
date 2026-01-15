#pragma once
#include <optional>
#include <string>

namespace Jwt {
  // Create a JWT for a given userId, with ttlSeconds expiry (HS256)
  std::string signUser(long userId, const std::string& secret, int ttlSeconds);

  // Verify token signature + exp and return userId if valid
  std::optional<long> verifyAndGetUserId(const std::string& token,
                                        const std::string& secret);
}
