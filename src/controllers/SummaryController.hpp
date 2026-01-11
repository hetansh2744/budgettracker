#pragma once

#include <memory>

#include "oatpp/web/server/api/ApiController.hpp"
#include "dto/SummaryDtos.hpp"
#include "services/SummaryService.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)

class SummaryController : public oatpp::web::server::api::ApiController {
 public:
  SummaryController(OATPP_COMPONENT(std::shared_ptr<oatpp::web::mime::ContentMappers>, apiContentMappers),
                    const std::shared_ptr<services::SummaryService>& summaryService)
      : oatpp::web::server::api::ApiController(apiContentMappers),
        summaryService_(summaryService) {}

  ENDPOINT("GET", "/summary/monthly", monthly,
           QUERY(Int32, year, "year"));

  ENDPOINT("GET", "/summary/by-category", byCategory,
           QUERY(String, type, "type"),
           QUERY(String, startDate, "startDate"),
           QUERY(String, endDate, "endDate"));

  ENDPOINT("GET", "/summary/cashflow", cashflow,
           QUERY(String, startDate, "startDate"),
           QUERY(String, endDate, "endDate"));

 private:
  std::shared_ptr<services::SummaryService> summaryService_;
};

#include OATPP_CODEGEN_END(ApiController)
