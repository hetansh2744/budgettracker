#include "Password.hpp"
#include <openssl/evp.h>
#include <openssl/rand.h>

#include <sstream>
#include <iomanip>
#include <vector>

static std::string toHex(const unsigned char* data, size_t len) {
  std::ostringstream oss;
  for (size_t i = 0; i < len; i++) {
    oss << std::hex << std::setw(2) << std::setfill('0') << (int)data[i];
  }
  return oss.str();
}

static std::vector<unsigned char> fromHex(const std::string& hex) {
  std::vector<unsigned char> out;
  if (hex.size() % 2 != 0) return out;
  out.reserve(hex.size() / 2);
  for (size_t i = 0; i < hex.size(); i += 2) {
    unsigned int byte = 0;
    std::stringstream ss;
    ss << std::hex << hex.substr(i, 2);
    ss >> byte;
    out.push_back((unsigned char)byte);
  }
  return out;
}

// Format:
// pbkdf2_sha256$ITER$saltHex$hashHex
std::string Password::hash(const std::string& password) {
  const int iter = 120000;
  unsigned char salt[16];
  if (RAND_bytes(salt, sizeof(salt)) != 1) {
    throw std::runtime_error("RAND_bytes failed");
  }

  unsigned char out[32];
  if (PKCS5_PBKDF2_HMAC(
        password.c_str(), (int)password.size(),
        salt, (int)sizeof(salt),
        iter,
        EVP_sha256(),
        (int)sizeof(out),
        out) != 1) {
    throw std::runtime_error("PBKDF2 failed");
  }

  std::ostringstream oss;
  oss << "pbkdf2_sha256$" << iter << "$"
      << toHex(salt, sizeof(salt)) << "$"
      << toHex(out, sizeof(out));
  return oss.str();
}

bool Password::verify(const std::string& password, const std::string& stored) {
  // Split by $
  size_t p1 = stored.find('$');
  if (p1 == std::string::npos) return false;
  size_t p2 = stored.find('$', p1 + 1);
  if (p2 == std::string::npos) return false;
  size_t p3 = stored.find('$', p2 + 1);
  if (p3 == std::string::npos) return false;

  std::string alg = stored.substr(0, p1);
  std::string iterStr = stored.substr(p1 + 1, p2 - (p1 + 1));
  std::string saltHex = stored.substr(p2 + 1, p3 - (p2 + 1));
  std::string hashHex = stored.substr(p3 + 1);

  if (alg != "pbkdf2_sha256") return false;

  int iter = 0;
  try { iter = std::stoi(iterStr); } catch (...) { return false; }
  if (iter < 10000) return false;

  auto salt = fromHex(saltHex);
  auto expected = fromHex(hashHex);
  if (salt.empty() || expected.empty()) return false;

  std::vector<unsigned char> out(expected.size());
  if (PKCS5_PBKDF2_HMAC(
        password.c_str(), (int)password.size(),
        salt.data(), (int)salt.size(),
        iter,
        EVP_sha256(),
        (int)out.size(),
        out.data()) != 1) {
    return false;
  }

  // Constant-time compare
  if (out.size() != expected.size()) return false;
  unsigned char diff = 0;
  for (size_t i = 0; i < out.size(); i++) diff |= (out[i] ^ expected[i]);
  return diff == 0;
}
