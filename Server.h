#pragma once
#include <netinet/in.h>
#include "ConfigHandler.h"
#include "Logger.h"
#include "UserBase.h"
#include "Authenticator.h"
#include "DataProcessor.h"

class Server {
private:
    int server_socket;
    uint16_t port;
    ConfigHandler config;
    Logger logger;
    UserBase users;
    Authenticator auth;
    DataProcessor calc;
public:
    Server(const ConfigHandler& cfg);
    void start();
};
