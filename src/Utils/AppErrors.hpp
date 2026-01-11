#pragma once

#include <stdexcept>
#include <string>

namespace utils {

enum class ErrorCode {
  VALIDATION_ERROR,
  UNAUTHORIZED,
  FORBIDDEN,
  NOT_FOUND,
  CONFLICT,
  INTERNAL_ERROR
};

struct AppError : public std::runtime_error {
  int httpStatus;
  ErrorCode code;
  std::string detailsJson;  // optional extra details as JSON string

  AppError(int status, ErrorCode c, const std::string& message,
           const std::string& details = "{}")
      : std::runtime_error(message), httpStatus(status), code(c), detailsJson(details) {}
};

std::string errorCodeToString(ErrorCode code);

}  // namespace utils
