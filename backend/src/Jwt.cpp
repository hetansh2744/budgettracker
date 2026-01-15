#include "Jwt.hpp"

#include <nlohmann/json.hpp>
#include <openssl/evp.h>
#include <openssl/hmac.h>

#include <ctime>
#include <optional>
#include <string>
#include <vector>

using json = nlohmann::json;

static std::string b64UrlEncodeRaw(const unsigned char* data, size_t len) {
  // Base64 encode -> then URL-safe transform
  int outLen = 4 * ((int(len) + 2) / 3);
  std::string out(outLen, '\0');

  EVP_EncodeBlock(reinterpret_cast<unsigned char*>(&out[0]), data,
                  static_cast<int>(len));

  for (char& c : out) {
    if (c == '+') c = '-';
    else if (c == '/') c = '_';
  }

  while (!out.empty() && out.back() == '=') out.pop_back();
  return out;
}

static std::string b64UrlEncode(const std::string& in) {
  return b64UrlEncodeRaw(reinterpret_cast<const unsigned char*>(in.data()),
                         in.size());
}

static std::string b64UrlDecodeToString(const std::string& in) {
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
  if (n < 0) return "";

  // trim padding bytes
  int pad = 0;
  if (!b64.empty() && b64.back() == '=') pad++;
  if (b64.size() > 1 && b64[b64.size() - 2] == '=') pad++;

  out.resize(static_cast<size_t>(n - pad));
  return std::string(out.begin(), out.end());
}

static std::string hmacSha256(const std::string& data,
                              const std::string& secret) {
  unsigned int len = 0;
  unsigned char out[EVP_MAX_MD_SIZE];

  HMAC(EVP_sha256(),
       secret.data(), static_cast<int>(secret.size()),
       reinterpret_cast<const unsigned char*>(data.data()),
       static_cast<int>(data.size()),
       out, &len);

  return std::string(reinterpret_cast<char*>(out), len);
}

static bool constantTimeEq(const std::string& a, const std::string& b) {
  if (a.size() != b.size()) return false;
  unsigned char diff = 0;
  for (size_t i = 0; i < a.size(); i++) {
    diff |= static_cast<unsigned char>(a[i] ^ b[i]);
  }
  return diff == 0;
}

std::string Jwt::signUser(long userId, const std::string& secret,
                          int ttlSeconds) {
  json header = {{"alg", "HS256"}, {"typ", "JWT"}};

  std::time_t now = std::time(nullptr);
  json payload = {
      {"sub", std::to_string(userId)},
      {"iat", static_cast<long>(now)},
      {"exp", static_cast<long>(now + ttlSeconds)},
  };

  std::string h = b64UrlEncode(header.dump());
  std::string p = b64UrlEncode(payload.dump());
  std::string signingInput = h + "." + p;

  std::string sigRaw = hmacSha256(signingInput, secret);
  std::string sig = b64UrlEncode(sigRaw);

  return signingInput + "." + sig;
}

std::optional<long> Jwt::verifyAndGetUserId(const std::string& token,
                                            const std::string& secret) {
  size_t a = token.find('.');
  if (a == std::string::npos) return std::nullopt;
  size_t b = token.find('.', a + 1);
  if (b == std::string::npos) return std::nullopt;

  std::string h = token.substr(0, a);
  std::string p = token.substr(a + 1, b - (a + 1));
  std::string s = token.substr(b + 1);

  std::string signingInput = h + "." + p;
  std::string expectedRaw = hmacSha256(signingInput, secret);
  std::string expected = b64UrlEncode(expectedRaw);

  if (!constantTimeEq(expected, s)) return std::nullopt;

  std::string payloadJson = b64UrlDecodeToString(p);
  if (payloadJson.empty()) return std::nullopt;

  json payload = json::parse(payloadJson, nullptr, false);
  if (payload.is_discarded()) return std::nullopt;

  if (!payload.contains("exp") || !payload.contains("sub")) return std::nullopt;

  long exp = 0;
  try {
    exp = payload["exp"].get<long>();
  } catch (...) {
    return std::nullopt;
  }

  std::time_t now = std::time(nullptr);
  if (now >= exp) return std::nullopt;

  std::string sub;
  try {
    sub = payload["sub"].get<std::string>();
  } catch (...) {
    return std::nullopt;
  }

  try {
    return std::stol(sub);
  } catch (...) {
    return std::nullopt;
  }
}
