#include "Server.h"
#include "ServerException.h"
#include <sys/socket.h>
#include <unistd.h>

Server::Server(const ConfigHandler& cfg)
    : server_socket(-1), port(cfg.getPort()), config(cfg),
      logger(cfg.getLogFile()), calc(users, auth, logger) {}

void Server::start() {
    try {
        users.load(config.getConfigFile());
        logger.write("База пользователей загружена.", false);
    } catch (const std::exception& e) {
        logger.write(e.what(), true);
        throw;
    }

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) throw ServerException("Ошибка создания сокета", true);

    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        close(server_socket);
        throw ServerException("Ошибка bind", true);
    }

    if (listen(server_socket, SOMAXCONN) < 0) {
        close(server_socket);
        throw ServerException("Ошибка listen", true);
    }

    logger.write("Сервер запущен на порту " + std::to_string(port), false);

    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_socket, reinterpret_cast<struct sockaddr*>(&client_addr), &client_len);
        if (client_socket < 0) continue;

        try {
            calc.process(client_socket);
        } catch (...) {
            close(client_socket);
        }
    }
}
