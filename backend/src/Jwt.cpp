#include "Jwt.hpp"
#include "nlohmann/json.hpp"

#include <openssl/hmac.h>

#include <ctime>
#include <string>
#include <vector>

using json = nlohmann::json;

static std::string b64UrlEncode(const std::string& in) {
  static const char* b64 =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
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

  for (char& c : out) {
    if (c == '+') c = '-';
    else if (c == '/') c = '_';
  }
  while (!out.empty() && out.back() == '=') out.pop_back();
  return out;
}

static std::string b64UrlDecode(const std::string& in) {
  std::string s = in;
  for (char& c : s) {
    if (c == '-') c = '+';
    else if (c == '_') c = '/';
  }
  while (s.size() % 4) s.push_back('=');

  static const int T[256] = {
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,
    52,53,54,55,56,57,58,59,60,61,-1,-1,-1, 0,-1,-1,
    -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
    15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
    -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
    41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
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

static std::string hmacSha256(const std::string& msg, const std::string& key) {
  unsigned int len = 0;
  unsigned char out[EVP_MAX_MD_SIZE];
  HMAC(EVP_sha256(),
       key.data(), static_cast<int>(key.size()),
       reinterpret_cast<const unsigned char*>(msg.data()),
       msg.size(),
       out, &len);
  return std::string(reinterpret_cast<char*>(out), len);
}

static bool constantTimeEqStr(const std::string& a, const std::string& b) {
  if (a.size() != b.size()) return false;
  unsigned char diff = 0;
  for (size_t i = 0; i < a.size(); i++) diff |= (a[i] ^ b[i]);
  return diff == 0;
}

std::string Jwt::signUser(long userId, const std::string& secret, int ttlSeconds) {
  json header = {{"alg", "HS256"}, {"typ", "JWT"}};
  std::time_t now = std::time(nullptr);
  json payload = {{"sub", userId}, {"iat", now}, {"exp", now + ttlSeconds}};

  std::string h = b64UrlEncode(header.dump());
  std::string p = b64UrlEncode(payload.dump());
  std::string msg = h + "." + p;

  std::string sig = hmacSha256(msg, secret);
  std::string s = b64UrlEncode(sig);
  return msg + "." + s;
}

std::optional<long> Jwt::verifyAndGetUserId(const std::string& token,
                                           const std::string& secret) {
  auto firstDot = token.find('.');
  auto secondDot = token.find('.', firstDot + 1);
  if (firstDot == std::string::npos || secondDot == std::string::npos) return std::nullopt;

  std::string h = token.substr(0, firstDot);
  std::string p = token.substr(firstDot + 1, secondDot - firstDot - 1);
  std::string s = token.substr(secondDot + 1);

  std::string msg = h + "." + p;
  std::string expected = b64UrlEncode(hmacSha256(msg, secret));
  if (!constantTimeEqStr(expected, s)) return std::nullopt;

  json payload = json::parse(b64UrlDecode(p), nullptr, false);
  if (payload.is_discarded()) return std::nullopt;

  std::time_t now = std::time(nullptr);
  if (!payload.contains("exp") || payload["exp"].get<long>() < now) return std::nullopt;
  if (!payload.contains("sub")) return std::nullopt;

  return payload["sub"].get<long>();
}
