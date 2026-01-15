#pragma once
#include <string>
#include <optional>

namespace Jwt {
  std::string signUser(long userId, const std::string& secret, int ttlSeconds);
  std::optional<long> verifyAndGetUserId(const std::string& token, const std::string& secret);
}
