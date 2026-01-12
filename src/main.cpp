#include "httplib.h"
#include "json.hpp"
#include "db/Database.hpp"
#include "repositories/SqliteTransactionRepository.hpp"
#include "services/FinanceService.hpp"
#include "api/FinanceRoutes.hpp"

int main() {
    Database db("tracker.db");
    db.runMigrations("db/migrations.sql");

    auto repo = std::make_shared<SqliteTransactionRepository>(db);
    FinanceService finance(repo);

    httplib::Server server;
    registerFinanceRoutes(server, finance);

    server.listen("0.0.0.0", 8080);
}
