#pragma once
#include <vector>
#include <optional>
#include "../models/Issue.hpp"

class IssueRepository {
public:
    virtual ~IssueRepository() = default;

    virtual void save(const Issue& issue) = 0;
    virtual std::vector<Issue> findAll() const = 0;
    virtual std::optional<Issue> findById(int id) const = 0;
};
