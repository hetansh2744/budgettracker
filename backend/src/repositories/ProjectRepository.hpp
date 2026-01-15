#pragma once
#include <vector>
#include <optional>
#include "../models/Project.hpp"

class ProjectRepository {
public:
    virtual ~ProjectRepository() = default;

    virtual void save(const Project& project) = 0;
    virtual std::vector<Project> findAll() const = 0;
};
