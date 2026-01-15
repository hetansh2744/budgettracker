#pragma once
#include <string>

namespace Env {
  std::string get(const std::string& key, const std::string& def = "");
  int getInt(const std::string& key, int def);
}
