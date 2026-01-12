#pragma once
#include "ProjectRepository.hpp"
#include "../db/Database.hpp"

class SqliteProjectRepository : public ProjectRepository {
public:
    explicit SqliteProjectRepository(Database& db);

    void addProject(const Project& project) override;
    std::vector<Project> findAll() override;

private:
    Database& database;
};
