#pragma once
#include <vector>
#include <memory>
#include "../models/Issue.hpp"
#include "../repositories/IssueRepository.hpp"

class IssueService {
private:
    std::shared_ptr<IssueRepository> repository;
    int nextId = 1;

public:
    explicit IssueService(std::shared_ptr<IssueRepository> repo);

    Issue createIssue(const std::string& title,
                      const std::string& description,
                      Priority priority);

    std::vector<Issue> getAllIssues() const;
};
