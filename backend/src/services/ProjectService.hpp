#pragma once
#include <vector>
#include <memory>
#include "../models/Project.hpp"
#include "../repositories/ProjectRepository.hpp"

class ProjectService {
private:
    std::shared_ptr<ProjectRepository> repository;
    int nextId = 1;

public:
    explicit ProjectService(std::shared_ptr<ProjectRepository> repo);

    Project createProject(const std::string& name);
    std::vector<Project> getAllProjects() const;
};
