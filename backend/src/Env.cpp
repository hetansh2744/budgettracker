#include "Env.hpp"
#include <cstdlib>

std::string Env::get(const std::string& key, const std::string& def) {
  const char* v = std::getenv(key.c_str());
  return v ? std::string(v) : def;
}

int Env::getInt(const std::string& key, int def) {
  const char* v = std::getenv(key.c_str());
  if (!v) return def;
  try { return std::stoi(v); } catch (...) { return def; }
}
