#include "IssueService.hpp"

IssueService::IssueService(std::shared_ptr<IssueRepository> repo)
    : repository(repo) {}

Issue IssueService::createIssue(const std::string& title,
                                const std::string& description,
                                Priority priority) {
    Issue issue(nextId++, title, description, priority);
    repository->save(issue);
    return issue;
}

std::vector<Issue> IssueService::getAllIssues() const {
    return repository->findAll();
}
