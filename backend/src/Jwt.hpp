#pragma once
#include <optional>
#include <string>

namespace Jwt {
std::string signUser(long userId, const std::string& secret, int ttlSeconds);
std::optional<long> verifyAndGetUserId(const std::string& token,
                                      const std::string& secret);
}  // namespace Jwt
