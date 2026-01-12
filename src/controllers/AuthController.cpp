#include "AuthController.hpp"

ENDPOINT("POST", "/auth/register", register,
         BODY_DTO(RegisterRequestDto, body)) {
  return createDtoResponse(
    Status::CODE_201, m_authService->registerUser(body));
}

ENDPOINT("POST", "/auth/login", login,
         BODY_DTO(LoginRequestDto, body)) {
  return createDtoResponse(
    Status::CODE_200, m_authService->login(body));
}
