#pragma once

#include <cstdint>
#include <string>

namespace repo {

struct User {
  std::string id;
  std::string name;
  std::string email;
  std::string passwordHash;
  std::string createdAt;
};

struct Category {
  std::string id;
  std::string userId;
  std::string name;
  std::string type; // INCOME/EXPENSE
  std::string createdAt;
};

struct Transaction {
  std::string id;
  std::string userId;
  std::string categoryId;
  std::string type; // INCOME/EXPENSE
  int64_t amountCents;
  std::string currency;
  std::string date; // YYYY-MM-DD
  std::string sourceOrMerchant;
  std::string note;
  std::string createdAt;
  std::string updatedAt;
};

struct RefreshToken {
  std::string id;
  std::string userId;
  std::string tokenHash;
  std::string revokedAt; // empty if not revoked
  std::string createdAt;
};

}  // namespace repo
