#include "Password.hpp"

#include <openssl/evp.h>
#include <openssl/rand.h>

#include <cstring>
#include <sstream>
#include <stdexcept>
#include <vector>

static std::string hexEncode(const unsigned char* data, size_t len) {
  static const char* kHex = "0123456789abcdef";
  std::string out;
  out.reserve(len * 2);
  for (size_t i = 0; i < len; i++) {
    unsigned char b = data[i];
    out.push_back(kHex[b >> 4]);
    out.push_back(kHex[b & 0x0F]);
  }
  return out;
}

static std::vector<unsigned char> hexDecode(const std::string& hex) {
  if (hex.size() % 2 != 0) throw std::runtime_error("hex decode: odd length");
  auto val = [](char c) -> int {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return -1;
  };

  std::vector<unsigned char> out(hex.size() / 2);
  for (size_t i = 0; i < out.size(); i++) {
    int hi = val(hex[2 * i]);
    int lo = val(hex[2 * i + 1]);
    if (hi < 0 || lo < 0) throw std::runtime_error("hex decode: bad char");
    out[i] = static_cast<unsigned char>((hi << 4) | lo);
  }
  return out;
}

static bool constantTimeEq(const std::vector<unsigned char>& a,
                           const std::vector<unsigned char>& b) {
  if (a.size() != b.size()) return false;
  unsigned char diff = 0;
  for (size_t i = 0; i < a.size(); i++) diff |= (a[i] ^ b[i]);
  return diff == 0;
}

// Stored format: pbkdf2$<iters>$<saltHex>$<dkHex>
std::string Password::hash(const std::string& password) {
  const int iters = 120000;
  unsigned char salt[16];
  if (RAND_bytes(salt, sizeof(salt)) != 1) {
    throw std::runtime_error("RAND_bytes failed");
  }

  unsigned char dk[32];
  if (PKCS5_PBKDF2_HMAC(password.c_str(),
                        static_cast<int>(password.size()),
                        salt, sizeof(salt),
                        iters,
                        EVP_sha256(),
                        sizeof(dk),
                        dk) != 1) {
    throw std::runtime_error("PBKDF2 failed");
  }

  std::ostringstream oss;
  oss << "pbkdf2$" << iters << "$" << hexEncode(salt, sizeof(salt)) << "$"
      << hexEncode(dk, sizeof(dk));
  return oss.str();
}

bool Password::verify(const std::string& password, const std::string& stored) {
  // split by $
  std::vector<std::string> parts;
  std::string cur;
  for (char c : stored) {
    if (c == '$') { parts.push_back(cur); cur.clear(); }
    else cur.push_back(c);
  }
  parts.push_back(cur);

  if (parts.size() != 4 || parts[0] != "pbkdf2") return false;

  int iters = 0;
  try { iters = std::stoi(parts[1]); } catch (...) { return false; }

  std::vector<unsigned char> salt;
  std::vector<unsigned char> dkStored;
  try {
    salt = hexDecode(parts[2]);
    dkStored = hexDecode(parts[3]);
  } catch (...) {
    return false;
  }

  std::vector<unsigned char> dk(dkStored.size());
  if (PKCS5_PBKDF2_HMAC(password.c_str(),
                        static_cast<int>(password.size()),
                        salt.data(), static_cast<int>(salt.size()),
                        iters,
                        EVP_sha256(),
                        static_cast<int>(dk.size()),
                        dk.data()) != 1) {
    return false;
  }

  return constantTimeEq(dk, dkStored);
}
