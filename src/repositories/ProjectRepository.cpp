#include "ProjectRepository.hpp"

class InMemoryProjectRepository : public ProjectRepository {
private:
    std::vector<Project> projects;

public:
    void save(const Project& project) override {
        projects.push_back(project);
    }

    std::vector<Project> findAll() const override {
        return projects;
    }
};
