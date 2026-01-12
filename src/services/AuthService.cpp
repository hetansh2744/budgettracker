#include "AuthService.hpp"

AuthResponseDto::ObjectWrapper
AuthService::registerUser(const RegisterRequestDto::ObjectWrapper& dto) {
  if (m_userRepo->findByEmail(dto->email)) {
    throw std::runtime_error("Email already exists");
  }

  User user;
  user.id = uuid();
  user.name = dto->name;
  user.email = dto->email;
  user.passwordHash = PasswordHasher::hash(dto->password);

  m_userRepo->create(user);

  auto response = AuthResponseDto::createShared();
  response->accessToken = m_jwtUtil.generateToken(user.id);
  return response;
}

AuthResponseDto::ObjectWrapper
AuthService::login(const LoginRequestDto::ObjectWrapper& dto) {
  auto userOpt = m_userRepo->findByEmail(dto->email);
  if (!userOpt ||
      !PasswordHasher::verify(dto->password, userOpt->passwordHash)) {
    throw std::runtime_error("Invalid credentials");
  }

  auto response = AuthResponseDto::createShared();
  response->accessToken = m_jwtUtil.generateToken(userOpt->id);
  return response;
}
