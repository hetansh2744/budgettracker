#pragma once

#include <optional>
#include <string>

#include "repositories/Models.hpp"

namespace repo {

class ITokenRepository {
 public:
  virtual ~ITokenRepository() = default;

  virtual RefreshToken create(const std::string& userId, const std::string& tokenHash) = 0;

  virtual std::optional<RefreshToken> findByIdForUser(const std::string& userId,
                                                      const std::string& tokenId) = 0;

  virtual void revoke(const std::string& userId, const std::string& tokenId) = 0;
};

}  // namespace repo
