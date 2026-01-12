#include "ProjectService.hpp"

ProjectService::ProjectService(std::shared_ptr<ProjectRepository> repo)
    : repository(repo) {}

Project ProjectService::createProject(const std::string& name) {
    Project project(nextId++, name);
    repository->save(project);
    return project;
}

std::vector<Project> ProjectService::getAllProjects() const {
    return repository->findAll();
}
