#pragma once

#include <memory>

#include "oatpp/web/server/api/ApiController.hpp"
#include "dto/CategoryDtos.hpp"
#include "services/CategoryService.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)

class CategoryController : public oatpp::web::server::api::ApiController {
 public:
  CategoryController(OATPP_COMPONENT(std::shared_ptr<oatpp::web::mime::ContentMappers>, apiContentMappers),
                     const std::shared_ptr<services::CategoryService>& categoryService)
      : oatpp::web::server::api::ApiController(apiContentMappers),
        categoryService_(categoryService) {}

  ENDPOINT("GET", "/categories", listCategories,
           QUERY(String, type, "type"));

  ENDPOINT("POST", "/categories", createCategory,
           BODY_DTO(Object<CategoryCreateDto>, body));

  ENDPOINT("PUT", "/categories/{id}", updateCategory,
           PATH(String, id),
           BODY_DTO(Object<CategoryUpdateDto>, body));

  ENDPOINT("DELETE", "/categories/{id}", deleteCategory,
           PATH(String, id));

 private:
  std::shared_ptr<services::CategoryService> categoryService_;
};

#include OATPP_CODEGEN_END(ApiController)
