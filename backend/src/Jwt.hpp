#pragma once
#include <optional>
#include <string>

namespace Jwt {
  // Sign a JWT for a user (HS256)
  std::string signUser(long userId, const std::string& secret, int ttlSeconds);

  // Verify JWT and return userId if valid (not expired, signature OK)
  std::optional<long> verifyAndGetUserId(const std::string& token,
                                        const std::string& secret);
}
