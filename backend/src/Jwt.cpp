#include "Jwt.hpp"
#include "json.hpp"

#include <openssl/hmac.h>
#include <ctime>
#include <vector>
#include <string>

using json = nlohmann::json;

static std::string b64urlEncode(const std::string& in) {
  static const char* b64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  std::string out;
  int val = 0, valb = -6;
  for (unsigned char c : in) {
    val = (val << 8) + c;
    valb += 8;
    while (valb >= 0) {
      out.push_back(b64[(val >> valb) & 0x3F]);
      valb -= 6;
    }
  }
  if (valb > -6) out.push_back(b64[((val << 8) >> (valb + 8)) & 0x3F]);
  while (out.size() % 4) out.push_back('=');

  // base64url transform
  for (auto& ch : out) {
    if (ch == '+') ch = '-';
    else if (ch == '/') ch = '_';
  }
  while (!out.empty() && out.back() == '=') out.pop_back();
  return out;
}

static std::string b64urlDecode(const std::string& in) {
  std::string s = in;
  for (auto& ch : s) {
    if (ch == '-') ch = '+';
    else if (ch == '_') ch = '/';
  }
  while (s.size() % 4) s.push_back('=');

  static const int T[256] = {
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,
    -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
    -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
  };

  std::string out;
  int val = 0, valb = -8;
  for (unsigned char c : s) {
    if (T[c] == -1) break;
    val = (val << 6) + T[c];
    valb += 6;
    if (valb >= 0) {
      out.push_back(char((val >> valb) & 0xFF));
      valb -= 8;
    }
  }
  return out;
}

static std::string hmacSha256(const std::string& data, const std::string& secret) {
  unsigned int len = 0;
  unsigned char out[EVP_MAX_MD_SIZE];
  HMAC(EVP_sha256(),
       secret.data(), (int)secret.size(),
       (const unsigned char*)data.data(), data.size(),
       out, &len);
  return std::string((char*)out, (char*)out + len);
}

std::string Jwt::signUser(long userId, const std::string& secret, int ttlSeconds) {
  json header = {{"alg","HS256"},{"typ","JWT"}};
  long now = (long)std::time(nullptr);
  json payload = {
    {"sub", userId},
    {"iat", now},
    {"exp", now + ttlSeconds}
  };

  std::string h = b64urlEncode(header.dump());
  std::string p = b64urlEncode(payload.dump());
  std::string toSign = h + "." + p;

  std::string sig = b64urlEncode(hmacSha256(toSign, secret));
  return toSign + "." + sig;
}

std::optional<long> Jwt::verifyAndGetUserId(const std::string& token, const std::string& secret) {
  size_t a = token.find('.');
  if (a == std::string::npos) return std::nullopt;
  size_t b = token.find('.', a + 1);
  if (b == std::string::npos) return std::nullopt;

  std::string h = token.substr(0, a);
  std::string p = token.substr(a + 1, b - (a + 1));
  std::string s = token.substr(b + 1);

  std::string toSign = h + "." + p;
  std::string expected = b64urlEncode(hmacSha256(toSign, secret));

  if (expected.size() != s.size()) return std::nullopt;
  unsigned char diff = 0;
  for (size_t i = 0; i < s.size(); i++) diff |= (unsigned char)(expected[i] ^ s[i]);
  if (diff != 0) return std::nullopt;

  json payload = json::parse(b64urlDecode(p), nullptr, false);
  if (payload.is_discarded()) return std::nullopt;

  long exp = payload.value("exp", 0L);
  long now = (long)std::time(nullptr);
  if (exp <= now) return std::nullopt;

  if (!payload.contains("sub")) return std::nullopt;
  return payload["sub"].get<long>();
}
