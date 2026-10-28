#include "ServerException.h"

ServerException::ServerException(std::string message, bool isCritical)
    : msg(std::move(message)), critical(isCritical) {}

const char* ServerException::what() const noexcept {
    return msg.c_str();
}
