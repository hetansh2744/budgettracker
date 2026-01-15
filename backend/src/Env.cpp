#include "Env.hpp"
#include <cstdlib>

std::string Env::get(const std::string& key, const std::string& def) {
  const char* v = std::getenv(key.c_str());
  return v ? std::string(v) : def;
}

int Env::getInt(const std::string& key, int def) {
  const char* v = std::getenv(key.c_str());
  if (!v) return def;
  try {
    return std::stoi(v);
  } catch (...) {
    return def;
  }
}

std::string Env::firstOf(std::initializer_list<std::string> keys,
                         const std::string& def) {
  for (const auto& k : keys) {
    auto v = get(k, "");
    if (!v.empty()) return v;
  }
  return def;
}
