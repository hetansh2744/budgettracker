#pragma once

#include <string>

namespace utils {

// Returns current UTC time ISO8601: "2026-01-09T18:30:00Z"
std::string nowUtcIso();

// Parses "YYYY-MM-DD" into a normalized string (returns same if valid).
// Throws std::invalid_argument if invalid.
std::string normalizeDate(const std::string& yyyy_mm_dd);

}  // namespace utils
