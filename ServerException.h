#pragma once
#include <exception>
#include <string>

class ServerException : public std::exception {
private:
    std::string msg;
    bool critical;
public:
    ServerException(std::string message, bool isCritical);
    const char* what() const noexcept override;
    bool isCritical() const { return critical; }
};
