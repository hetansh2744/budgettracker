#include "IssueRepository.hpp"
#include <algorithm>

class InMemoryIssueRepository : public IssueRepository {
private:
    std::vector<Issue> issues;

public:
    void save(const Issue& issue) override {
        issues.push_back(issue);
    }

    std::vector<Issue> findAll() const override {
        return issues;
    }

    std::optional<Issue> findById(int id) const override {
        auto it = std::find_if(
            issues.begin(), issues.end(),
            [id](const Issue& i) { return i.getId() == id; }
        );

        if (it != issues.end())
            return *it;

        return std::nullopt;
    }
};
