#pragma once

#include <memory>
#include <string>

#include "dto/AuthDtos.hpp"
#include "repositories/IUserRepository.hpp"
#include "repositories/ITokenRepository.hpp"
#include "utils/PasswordHasher.hpp"
#include "config/Env.hpp"

namespace services {

class AuthService {
 public:
  AuthService(const std::shared_ptr<repo::IUserRepository>& userRepo,
              const std::shared_ptr<repo::ITokenRepository>& tokenRepo,
              const utils::PasswordHasher& hasher,
              const config::Env& env);

  oatpp::Object<AuthResponseDto> registerUser(const oatpp::Object<RegisterRequestDto>& dto);
  oatpp::Object<AuthResponseDto> login(const oatpp::Object<LoginRequestDto>& dto);

  // Optional refresh flow
  oatpp::Object<AuthResponseDto> refresh(const std::string& userIdFromRefresh);

 private:
  std::shared_ptr<repo::IUserRepository> userRepo_;
  std::shared_ptr<repo::ITokenRepository> tokenRepo_;
  utils::PasswordHasher hasher_;
  config::Env env_;
};

}  // namespace services
