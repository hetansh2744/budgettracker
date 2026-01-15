#pragma once

#include <cstdint>
#include <string>

namespace utils {

// Convert decimal string "19.99" -> 1999 cents (CAD) etc.
// We'll keep cents in DB to avoid floating point bugs.
int64_t parseAmountToCents(const std::string& amount);

// Convert cents -> "19.99"
std::string centsToAmountString(int64_t cents);

}  // namespace utils
