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

static std::vector<unsigned char> fromHex(const std::string& s) {
  std::vector<unsigned char> out;
  out.reserve(s.size() / 2);
  for (size_t i = 0; i + 1 < s.size(); i += 2) {
    unsigned int x = 0;
    std::stringstream ss;
    ss << std::hex << s.substr(i, 2);
    ss >> x;
    out.push_back((unsigned char)x);
  }
  return out;
}

std::string Password::hash(const std::string& password) {
  const int iters = 200000;
  unsigned char salt[16];
  RAND_bytes(salt, sizeof(salt));

  unsigned char out[32];
  PKCS5_PBKDF2_HMAC(password.c_str(), (int)password.size(),
                    salt, (int)sizeof(salt),
                    iters, EVP_sha256(),
                    (int)sizeof(out), out);

  std::ostringstream oss;
  oss << "pbkdf2$" << iters << "$" << toHex(salt, sizeof(salt)) << "$" << toHex(out, sizeof(out));
  return oss.str();
}

bool Password::verify(const std::string& password, const std::string& stored) {
  auto p1 = stored.find('$');
  auto p2 = stored.find('$', p1 + 1);
  auto p3 = stored.find('$', p2 + 1);
  if (p1 == std::string::npos || p2 == std::string::npos || p3 == std::string::npos) return false;

  int iters = 0;
  try { iters = std::stoi(stored.substr(p1 + 1, p2 - (p1 + 1))); }
  catch (...) { return false; }

  std::string saltHex = stored.substr(p2 + 1, p3 - (p2 + 1));
  std::string hashHex = stored.substr(p3 + 1);

  auto salt = fromHex(saltHex);
  auto expected = fromHex(hashHex);
  if (expected.size() != 32 || salt.empty()) return false;

  unsigned char out[32];
  PKCS5_PBKDF2_HMAC(password.c_str(), (int)password.size(),
                    salt.data(), (int)salt.size(),
                    iters, EVP_sha256(),
                    (int)sizeof(out), out);

  unsigned int diff = 0;
  for (size_t i = 0; i < 32; i++) diff |= (out[i] ^ expected[i]);
  return diff == 0;
}
