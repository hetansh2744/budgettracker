#pragma once

#include <optional>
#include <string>

#include "repositories/Models.hpp"

namespace repo {

class IUserRepository {
 public:
  virtual ~IUserRepository() = default;

  virtual std::optional<User> findByEmail(const std::string& email) = 0;
  virtual std::optional<User> findById(const std::string& userId) = 0;

  virtual User createUser(const std::string& name,
                          const std::string& email,
                          const std::string& passwordHash) = 0;
};

}  // namespace repo
