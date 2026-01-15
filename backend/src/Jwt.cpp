#include "Jwt.hpp"
#include "json.hpp"
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <ctime>
#include <sstream>

using json = nlohmann::json;

static std::string b64urlEncode(const std::string& in) {
  std::string out;
  out.resize(4 * ((in.size() + 2) / 3));
  int len = EVP_EncodeBlock((unsigned char*)&out[0], (const unsigned char*)in.data(), (int)in.size());
  out.resize(len);
  for (char& c : out) { if (c == '+') c = '-'; else if (c == '/') c = '_'; }
  while (!out.empty() && out.back() == '=') out.pop_back();
  return out;
}

static std::string b64urlToB64(std::string s) {
  for (char& c : s) { if (c == '-') c = '+'; else if (c == '_') c = '/'; }
  while (s.size() % 4) s.push_back('=');
  return s;
}

static std::string b64decode(const std::string& b64) {
  std::string out;
  out.resize((b64.size() * 3) / 4);
  int len = EVP_DecodeBlock((unsigned char*)&out[0], (const unsigned char*)b64.data(), (int)b64.size());
  if (len < 0) return {};
  out.resize(len);
  while (!out.empty() && out.back() == '\0') out.pop_back();
  return out;
}

static std::string hmac256(const std::string& data, const std::string& secret) {
  unsigned int len = 0;
  unsigned char* digest = HMAC(EVP_sha256(),
                               secret.data(), (int)secret.size(),
                               (const unsigned char*)data.data(), data.size(),
                               nullptr, &len);
  return std::string((char*)digest, (char*)digest + len);
}

static bool timingEq(const std::string& a, const std::string& b) {
  if (a.size() != b.size()) return false;
  unsigned char diff = 0;
  for (size_t i = 0; i < a.size(); i++) diff |= (unsigned char)(a[i] ^ b[i]);
  return diff == 0;
}

std::string Jwt::signUser(long userId, const std::string& secret, int ttlSeconds) {
  json header = {{"alg","HS256"},{"typ","JWT"}};
  std::time_t now = std::time(nullptr);
  json payload = {{"sub", userId}, {"iat", (long)now}, {"exp", (long)(now + ttlSeconds)}};

  std::string h = b64urlEncode(header.dump());
  std::string p = b64urlEncode(payload.dump());
  std::string msg = h + "." + p;

  std::string sig = b64urlEncode(hmac256(msg, secret));
  return msg + "." + sig;
}

std::optional<long> Jwt::verifyAndGetUserId(const std::string& token, const std::string& secret) {
  auto a = token.find('.');
  auto b = token.find('.', a + 1);
  if (a == std::string::npos || b == std::string::npos) return std::nullopt;

  std::string h = token.substr(0, a);
  std::string p = token.substr(a + 1, b - (a + 1));
  std::string s = token.substr(b + 1);

  std::string msg = h + "." + p;
  std::string expected = b64urlEncode(hmac256(msg, secret));
  if (!timingEq(expected, s)) return std::nullopt;

  std::string payloadJson = b64decode(b64urlToB64(p));
  if (payloadJson.empty()) return std::nullopt;

  json payload = json::parse(payloadJson, nullptr, false);
  if (payload.is_discarded()) return std::nullopt;

  long exp = payload.value("exp", 0L);
  long sub = payload.value("sub", 0L);
  long now = (long)std::time(nullptr);

  if (sub <= 0 || exp <= now) return std::nullopt;
  return sub;
}
