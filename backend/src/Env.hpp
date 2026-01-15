#pragma once
#include <string>
#include <initializer_list>

namespace Env {
  std::string get(const std::string& key, const std::string& def = "");
  int getInt(const std::string& key, int def);

  std::string firstOf(std::initializer_list<std::string> keys,
                      const std::string& def = "");
}
