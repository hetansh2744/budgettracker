#include "oatpp/core/base/Environment.hpp"
#include "oatpp/network/Server.hpp"

#include "AppComponent.hpp"
#include "api/IssueController.hpp"
#include "api/ProjectController.hpp"

#include "repositories/IssueRepository.hpp"
#include "repositories/ProjectRepository.hpp"
#include "services/IssueService.hpp"
#include "services/ProjectService.hpp"

int main() {
    oatpp::base::Environment::init();

    AppComponent components;

    auto issueRepo = std::make_shared<InMemoryIssueRepository>();
    auto projectRepo = std::make_shared<InMemoryProjectRepository>();

    auto issueService = std::make_shared<IssueService>(issueRepo);
    auto projectService = std::make_shared<ProjectService>(projectRepo);

    OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);

    router->addController(std::make_shared<IssueController>(nullptr, issueService));
    router->addController(std::make_shared<ProjectController>(nullptr, projectService));

    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, connectionProvider);
    OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpConnectionHandler>, connectionHandler);

    oatpp::network::Server server(connectionProvider, connectionHandler);
    server.run();

    oatpp::base::Environment::destroy();
    return 0;
}
