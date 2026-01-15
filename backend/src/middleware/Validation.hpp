#pragma once

#include <string>

namespace middleware {

// Basic helpers. Deeper validation is in services.
void requireNonEmpty(const std::string& value, const std::string& fieldName);
void requireEmailLike(const std::string& email);
void requireTypeIncomeOrExpense(const std::string& type);
void requirePositiveCents(long long cents, const std::string& fieldName);

}  // namespace middleware
