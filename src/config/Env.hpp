#pragma once

#include <string>

namespace config {

struct Env {
  std::string dbPath;

  std::string jwtAccessSecret;
  int jwtAccessTtlSeconds;

  std::string jwtRefreshSecret;
  int jwtRefreshTtlSeconds;

  std::string apiHost;
};

Env loadEnv();

}  // namespace config
