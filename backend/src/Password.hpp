#pragma once
#include <string>

namespace Password {
  // Stored format: pbkdf2$iters$saltHex$hashHex
  std::string hash(const std::string& password);
  bool verify(const std::string& password, const std::string& stored);
}
