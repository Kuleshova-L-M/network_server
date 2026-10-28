#pragma once
#include <vector>
#include <cstdint>
#include "UserBase.h"
#include "Authenticator.h"
#include "Logger.h"

class DataProcessor {
private:
    UserBase& users;
    Authenticator& auth;
    Logger& logger;
    int32_t calcProduct(const std::vector<uint32_t>& data);
public:
    DataProcessor(UserBase& u, Authenticator& a, Logger& l);
    void process(int socket);
};
