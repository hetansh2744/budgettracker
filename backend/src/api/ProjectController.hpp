#pragma once

#include "oatpp/web/server/api/ApiController.hpp"
#include "../services/ProjectService.hpp"
#include "../dto/ProjectDTO.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)

class ProjectController : public oatpp::web::server::api::ApiController {
private:
    std::shared_ptr<ProjectService> projectService;

public:
    ProjectController(
        OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper),
        std::shared_ptr<ProjectService> service
    ) : oatpp::web::server::api::ApiController(objectMapper),
        projectService(service) {}

    ENDPOINT("GET", "/projects", getProjects) {
        auto projects = projectService->getAllProjects();
        auto response = oatpp::List<oatpp::Object<ProjectDTO>>::create();

        for (const auto& project : projects) {
            auto dto = ProjectDTO::createShared();
            dto->id = project.getId();
            dto->name = project.getName().c_str();
            response->push_back(dto);
        }

        return createDtoResponse(Status::CODE_200, response);
    }
};

#include OATPP_CODEGEN_END(ApiController)
