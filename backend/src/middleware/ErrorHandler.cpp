#include "ErrorHandler.hpp"

oatpp::web::protocol::http::OutgoingResponse*
ErrorHandler::handleError(const std::exception& e) {
  auto dto = ErrorDto::createShared();
  dto->code = "INTERNAL_ERROR";
  dto->message = e.what();

  return oatpp::web::protocol::http::ResponseFactory::createResponse(
    Status::CODE_500, dto);
}
