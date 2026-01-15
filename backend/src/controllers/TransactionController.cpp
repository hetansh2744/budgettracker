#include "TransactionController.hpp"

ENDPOINT("POST", "/transactions", create,
         BODY_DTO(TransactionCreateDto, body),
         REQUEST(std::shared_ptr<IncomingRequest>, request)) {
  auto userId = request->getAttribute<std::string>("userId");
  return createDtoResponse(
    Status::CODE_201,
    m_service->create(userId, body));
}

ENDPOINT("GET", "/transactions", list,
         REQUEST(std::shared_ptr<IncomingRequest>, request)) {
  auto userId = request->getAttribute<std::string>("userId");
  return createDtoResponse(
    Status::CODE_200,
    m_service->list(userId));
}
