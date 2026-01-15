#pragma once

#include <memory>

#include "oatpp/web/server/api/ApiController.hpp"
#include "dto/TransactionDtos.hpp"
#include "repositories/ITransactionRepository.hpp"
#include "services/TransactionService.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)

class TransactionController : public oatpp::web::server::api::ApiController {
 public:
  TransactionController(OATPP_COMPONENT(std::shared_ptr<oatpp::web::mime::ContentMappers>, apiContentMappers),
                        const std::shared_ptr<services::TransactionService>& txService)
      : oatpp::web::server::api::ApiController(apiContentMappers),
        txService_(txService) {}

  ENDPOINT("POST", "/transactions", createTx,
           BODY_DTO(Object<TransactionCreateDto>, body));

  ENDPOINT("GET", "/transactions", listTx,
           QUERY(String, type, "type", ""),
           QUERY(String, startDate, "startDate", ""),
           QUERY(String, endDate, "endDate", ""),
           QUERY(String, categoryId, "categoryId", ""),
           QUERY(Int64, minAmountCents, "minAmountCents", -1),
           QUERY(Int64, maxAmountCents, "maxAmountCents", -1),
           QUERY(Int32, page, "page", 1),
           QUERY(Int32, pageSize, "pageSize", 20),
           QUERY(String, sort, "sort", "date_desc"));

  ENDPOINT("GET", "/transactions/{id}", getTx,
           PATH(String, id));

  ENDPOINT("PUT", "/transactions/{id}", updateTx,
           PATH(String, id),
           BODY_DTO(Object<TransactionUpdateDto>, body));

  ENDPOINT("DELETE", "/transactions/{id}", deleteTx,
           PATH(String, id));

 private:
  std::shared_ptr<services::TransactionService> txService_;
};

#include OATPP_CODEGEN_END(ApiController)
