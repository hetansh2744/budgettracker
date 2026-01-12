#include <iostream>
#include <memory>

#include "httplib.h"
#include "json.hpp"

#include "db/Database.hpp"

#include "repositories/SqliteIssueRepository.hpp"
#include "repositories/SqliteProjectRepository.hpp"

#include "services/IssueService.hpp"
#include "services/ProjectService.hpp"

using json = nlohmann::json;

int main() {
    try {
        // --------------------------------------------------
        // 1. Database setup
        // --------------------------------------------------
        Database db("tracker.db");
        db.runMigrations("db/migrations.sql");

        // --------------------------------------------------
        // 2. Repositories
        // --------------------------------------------------
        auto issueRepository   = std::make_shared<SqliteIssueRepository>(db);
        auto projectRepository = std::make_shared<SqliteProjectRepository>(db);

        // --------------------------------------------------
        // 3. Services
        // --------------------------------------------------
        IssueService issueService(issueRepository);
        ProjectService projectService(projectRepository);

        // --------------------------------------------------
        // 4. HTTP Server
        // --------------------------------------------------
        httplib::Server server;

        // -------------------------
        // Health check
        // -------------------------
        server.Get("/health", [](const httplib::Request&, httplib::Response& res) {
            res.set_content(R"({"status":"ok"})", "application/json");
        });

        // -------------------------
        // Create Project
        // -------------------------
        server.Post("/projects", [&](const httplib::Request& req, httplib::Response& res) {
            try {
                auto body = json::parse(req.body);
                std::string name = body.at("name");

                auto project = projectService.createProject(name);

                json response = {
                    {"id", project.getId()},
                    {"name", project.getName()}
                };

                res.set_content(response.dump(), "application/json");
            } catch (...) {
                res.status = 400;
                res.set_content(R"({"error":"Invalid project data"})", "application/json");
            }
        });

        // -------------------------
        // List Projects
        // -------------------------
        server.Get("/projects", [&](const httplib::Request&, httplib::Response& res) {
            auto projects = projectService.getAllProjects();

            json response = json::array();
            for (const auto& p : projects) {
                response.push_back({
                    {"id", p.getId()},
                    {"name", p.getName()}
                });
            }

            res.set_content(response.dump(), "application/json");
        });

        // -------------------------
        // Create Issue
        // -------------------------
        server.Post("/issues", [&](const httplib::Request& req, httplib::Response& res) {
            try {
                auto body = json::parse(req.body);

                std::string title = body.at("title");
                std::string description = body.value("description", "");
                Priority priority = Priority::HIGH;

                auto issue = issueService.createIssue(title, description, priority);

                json response = {
                    {"id", issue.getId()},
                    {"title", issue.getTitle()},
                    {"description", issue.getDescription()},
                    {"status", "OPEN"}
                };

                res.set_content(response.dump(), "application/json");
            } catch (...) {
                res.status = 400;
                res.set_content(R"({"error":"Invalid issue data"})", "application/json");
            }
        });

        // -------------------------
        // List Issues
        // -------------------------
        server.Get("/issues", [&](const httplib::Request&, httplib::Response& res) {
            auto issues = issueService.getAllIssues();

            json response = json::array();
            for (const auto& i : issues) {
                response.push_back({
                    {"id", i.getId()},
                    {"title", i.getTitle()},
                    {"description", i.getDescription()},
                    {"status", "OPEN"}
                });
            }

            res.set_content(response.dump(), "application/json");
        });

        // --------------------------------------------------
        // 5. Start server
        // --------------------------------------------------
        std::cout << "🚀 Server running at http://localhost:8080\n";
        server.listen("0.0.0.0", 8080);

    } catch (const std::exception& ex) {
        std::cerr << "❌ Fatal error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
