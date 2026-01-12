#pragma once

#include "oatpp/web/server/api/ApiController.hpp"
#include "../services/IssueService.hpp"
#include "../dto/IssueDTO.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)

class IssueController : public oatpp::web::server::api::ApiController {
private:
    std::shared_ptr<IssueService> issueService;

public:
    IssueController(
        OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper),
        std::shared_ptr<IssueService> service
    ) : oatpp::web::server::api::ApiController(objectMapper),
        issueService(service) {}

    ENDPOINT("GET", "/issues", getIssues) {
        auto issues = issueService->getAllIssues();
        auto response = oatpp::List<oatpp::Object<IssueDTO>>::create();

        for (const auto& issue : issues) {
            auto dto = IssueDTO::createShared();
            dto->id = issue.getId();
            dto->title = issue.getTitle().c_str();
            dto->description = issue.getDescription().c_str();
            dto->priority = "HIGH";
            dto->status = "OPEN";
            response->push_back(dto);
        }

        return createDtoResponse(Status::CODE_200, response);
    }
};

#include OATPP_CODEGEN_END(ApiController)
