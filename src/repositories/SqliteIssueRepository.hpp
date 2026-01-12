#pragma once
#include "IssueRepository.hpp"
#include "../db/Database.hpp"

class SqliteIssueRepository : public IssueRepository {
public:
    explicit SqliteIssueRepository(Database& db);

    void addIssue(const Issue& issue) override;
    std::vector<Issue> findAll() override;

private:
    Database& database;
};
