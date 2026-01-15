#include "Password.hpp"

#include <openssl/evp.h>
#include <openssl/rand.h>

#include <cstring>
#include <sstream>
#include <string>
#include <vector>

static std::string b64UrlEncode(const std::vector<unsigned char>& in) {
  // Standard base64 then convert to base64url (no padding)
  int outLen = 4 * ((int(in.size()) + 2) / 3);
  std::string out(outLen, '\0');
  EVP_EncodeBlock(reinterpret_cast<unsigned char*>(&out[0]), in.data(),
                  static_cast<int>(in.size()));

  // base64 -> base64url
  for (char& c : out) {
    if (c == '+') c = '-';
    else if (c == '/') c = '_';
  }
  while (!out.empty() && out.back() == '=') out.pop_back();
  return out;
}

static std::vector<unsigned char> b64UrlDecode(const std::string& in) {
  std::string b64 = in;
  for (char& c : b64) {
    if (c == '-') c = '+';
    else if (c == '_') c = '/';
  }
  while (b64.size() % 4 != 0) b64.push_back('=');

  std::vector<unsigned char> out((b64.size() * 3) / 4);
  int n = EVP_DecodeBlock(out.data(),
                          reinterpret_cast<const unsigned char*>(b64.data()),
                          static_cast<int>(b64.size()));
  if (n < 0) return {};
  // EVP_DecodeBlock may include extra bytes due to padding; trim by counting '='
  int pad = 0;
  if (!b64.empty() && b64[b64.size() - 1] == '=') pad++;
  if (b64.size() > 1 && b64[b64.size() - 2] == '=') pad++;
  out.resize(n - pad);
  return out;
}

static bool constantTimeEq(const std::vector<unsigned char>& a,
                           const std::vector<unsigned char>& b) {
  if (a.size() != b.size()) return false;
  unsigned char diff = 0;
  for (size_t i = 0; i < a.size(); i++) diff |= (a[i] ^ b[i]);
  return diff == 0;
}

std::string Password::hash(const std::string& password) {
  const int iterations = 120000;
  const int saltLen = 16;
  const int hashLen = 32;

  std::vector<unsigned char> salt(saltLen);
  RAND_bytes(salt.data(), saltLen);

  std::vector<unsigned char> out(hashLen);
  PKCS5_PBKDF2_HMAC(password.c_str(), static_cast<int>(password.size()),
                    salt.data(), saltLen, iterations, EVP_sha256(),
                    hashLen, out.data());

  std::ostringstream ss;
  ss << "pbkdf2$" << iterations << "$" << b64UrlEncode(salt) << "$"
     << b64UrlEncode(out);
  return ss.str();
}

bool Password::verify(const std::string& password, const std::string& stored) {
  // stored format: pbkdf2$iters$salt$hash
  const std::string prefix = "pbkdf2$";
  if (stored.rfind(prefix, 0) != 0) return false;

  // Split by $
  size_t p1 = stored.find('$', prefix.size());
  if (p1 == std::string::npos) return false;
  size_t p2 = stored.find('$', p1 + 1);
  if (p2 == std::string::npos) return false;
  size_t p3 = stored.find('$', p2 + 1);
  if (p3 != std::string::npos) return false;  // should be exactly 4 parts

  std::string itersStr = stored.substr(prefix.size(), p1 - prefix.size());
  std::string saltB64 = stored.substr(p1 + 1, p2 - (p1 + 1));
  std::string hashB64 = stored.substr(p2 + 1);

  int iterations = 0;
  try { iterations = std::stoi(itersStr); } catch (...) { return false; }
  if (iterations < 10000) return false;

  auto salt = b64UrlDecode(saltB64);
  auto expected = b64UrlDecode(hashB64);
  if (salt.empty() || expected.empty()) return false;

  std::vector<unsigned char> out(expected.size());
  PKCS5_PBKDF2_HMAC(password.c_str(), static_cast<int>(password.size()),
                    salt.data(), static_cast<int>(salt.size()),
                    iterations, EVP_sha256(),
                    static_cast<int>(out.size()), out.data());

  return constantTimeEq(out, expected);
}
