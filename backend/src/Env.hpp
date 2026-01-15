#pragma once
#include <string>

namespace Env {
  std::string get(const std::string& key, const std::string& def = "");
  int getInt(const std::string& key, int def);

  // Helper: return first non-empty env var among keys
  std::string firstOf(std::initializer_list<std::string> keys,
                      const std::string& def = "");
}
