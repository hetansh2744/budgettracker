#pragma once

#include <optional>
#include <string>
#include <vector>

#include "repositories/Models.hpp"

namespace repo {

class ICategoryRepository {
 public:
  virtual ~ICategoryRepository() = default;

  virtual std::vector<Category> listForUser(const std::string& userId,
                                            const std::string& type /*optional empty*/) = 0;

  virtual std::optional<Category> findByIdForUser(const std::string& userId,
                                                  const std::string& categoryId) = 0;

  virtual Category create(const std::string& userId,
                          const std::string& name,
                          const std::string& type) = 0;

  virtual Category updateName(const std::string& userId,
                              const std::string& categoryId,
                              const std::string& newName) = 0;

  virtual void remove(const std::string& userId, const std::string& categoryId) = 0;
};

}  // namespace repo
