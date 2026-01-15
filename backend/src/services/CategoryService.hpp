#pragma once

#include <memory>
#include <string>
#include <vector>

#include "dto/CategoryDtos.hpp"
#include "repositories/ICategoryRepository.hpp"

namespace services {

class CategoryService {
 public:
  explicit CategoryService(const std::shared_ptr<repo::ICategoryRepository>& categoryRepo);

  oatpp::List<oatpp::Object<CategoryDto>> list(const std::string& userId,
                                              const std::string& type);

  oatpp::Object<CategoryDto> create(const std::string& userId,
                                    const oatpp::Object<CategoryCreateDto>& dto);

  oatpp::Object<CategoryDto> update(const std::string& userId,
                                    const std::string& categoryId,
                                    const oatpp::Object<CategoryUpdateDto>& dto);

  void remove(const std::string& userId, const std::string& categoryId);

 private:
  std::shared_ptr<repo::ICategoryRepository> categoryRepo_;
};

}  // namespace services
