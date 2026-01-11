#pragma once

#include <memory>

#include "oatpp/core/macro/component.hpp"
#include "oatpp/network/ServerConnectionProvider.hpp"
#include "oatpp/web/server/HttpConnectionHandler.hpp"
#include "oatpp/web/server/interceptor/RequestInterceptor.hpp"
#include "oatpp/web/mime/ContentMappers.hpp"

#include "config/Env.hpp"
#include "middleware/AuthMiddleware.hpp"
#include "repositories/sqlite/SQLiteDb.hpp"
#include "repositories/sqlite/SQLiteUserRepository.hpp"
#include "repositories/sqlite/SQLiteCategoryRepository.hpp"
#include "repositories/sqlite/SQLiteTransactionRepository.hpp"
#include "repositories/sqlite/SQLiteTokenRepository.hpp"
#include "services/AuthService.hpp"
#include "services/CategoryService.hpp"
#include "services/TransactionService.hpp"
#include "services/SummaryService.hpp"
#include "utils/PasswordHasher.hpp"

class AppComponent {
 public:
  explicit AppComponent(const config::Env& env);

  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>,
                        serverConnectionProvider)([] {
    return oatpp::network::tcp::server::ConnectionProvider::createShared({"0.0.0.0", 8000, oatpp::network::Address::IP_4});
  }());

  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::web::server::HttpConnectionHandler>,
                        httpConnectionHandler)([] {
    return oatpp::web::server::HttpConnectionHandler::createShared();
  }());

  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::web::mime::ContentMappers>,
                        apiContentMappers)([] {
    auto mappers = oatpp::web::mime::ContentMappers::createShared();
    mappers->putMapper("application/json", oatpp::parser::json::mapping::ObjectMapper::createShared());
    return mappers;
  }());

  // Environment
  OATPP_CREATE_COMPONENT(config::Env, env)([this] { return env_; }());

  // Auth middleware
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::web::server::interceptor::RequestInterceptor>,
                        authInterceptor)([this] {
    return std::make_shared<middleware::AuthMiddleware>(env_.jwtAccessSecret);
  }());

  // SQLite DB
  OATPP_CREATE_COMPONENT(std::shared_ptr<repo::sqlite::SQLiteDb>, sqliteDb)([this] {
    return std::make_shared<repo::sqlite::SQLiteDb>(env_.dbPath);
  }());

  // Repositories
  OATPP_CREATE_COMPONENT(std::shared_ptr<repo::IUserRepository>, userRepo)([] {
    OATPP_COMPONENT(std::shared_ptr<repo::sqlite::SQLiteDb>, db);
    return std::make_shared<repo::sqlite::SQLiteUserRepository>(db);
  }());

  OATPP_CREATE_COMPONENT(std::shared_ptr<repo::ICategoryRepository>, categoryRepo)([] {
    OATPP_COMPONENT(std::shared_ptr<repo::sqlite::SQLiteDb>, db);
    return std::make_shared<repo::sqlite::SQLiteCategoryRepository>(db);
  }());

  OATPP_CREATE_COMPONENT(std::shared_ptr<repo::ITransactionRepository>, txRepo)([] {
    OATPP_COMPONENT(std::shared_ptr<repo::sqlite::SQLiteDb>, db);
    return std::make_shared<repo::sqlite::SQLiteTransactionRepository>(db);
  }());

  OATPP_CREATE_COMPONENT(std::shared_ptr<repo::ITokenRepository>, tokenRepo)([] {
    OATPP_COMPONENT(std::shared_ptr<repo::sqlite::SQLiteDb>, db);
    return std::make_shared<repo::sqlite::SQLiteTokenRepository>(db);
  }());

  // Services
  OATPP_CREATE_COMPONENT(std::shared_ptr<services::AuthService>, authService)([this] {
    OATPP_COMPONENT(std::shared_ptr<repo::IUserRepository>, users);
    OATPP_COMPONENT(std::shared_ptr<repo::ITokenRepository>, tokens);
    utils::PasswordHasher hasher;
    return std::make_shared<services::AuthService>(users, tokens, hasher, env_);
  }());

  OATPP_CREATE_COMPONENT(std::shared_ptr<services::CategoryService>, categoryService)([] {
    OATPP_COMPONENT(std::shared_ptr<repo::ICategoryRepository>, cats);
    return std::make_shared<services::CategoryService>(cats);
  }());

  OATPP_CREATE_COMPONENT(std::shared_ptr<services::TransactionService>, txService)([] {
    OATPP_COMPONENT(std::shared_ptr<repo::ITransactionRepository>, tx);
    OATPP_COMPONENT(std::shared_ptr<repo::ICategoryRepository>, cats);
    return std::make_shared<services::TransactionService>(tx, cats);
  }());

  OATPP_CREATE_COMPONENT(std::shared_ptr<services::SummaryService>, summaryService)([] {
    OATPP_COMPONENT(std::shared_ptr<repo::ITransactionRepository>, tx);
    OATPP_COMPONENT(std::shared_ptr<repo::ICategoryRepository>, cats);
    return std::make_shared<services::SummaryService>(tx, cats);
  }());

 private:
  config::Env env_;
};
