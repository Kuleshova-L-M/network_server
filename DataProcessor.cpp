#include "DataProcessor.h"
#include <cstdint>
#include <string>
#include <vector>
#include <cstddef>
#include <unistd.h>
#include <sys/socket.h>


DataProcessor::DataProcessor(UserBase& u, Authenticator& a, Logger& l)
    : users(u), auth(a), logger(l) {}

int32_t DataProcessor::calcProduct(const std::vector<uint32_t>& data) {
    if (data.empty()) return 1;
    int64_t result = 1;
    int64_t max_limit = 2147483647LL;  // 2^31 - 1
    int64_t min_limit = -2147483648LL; // -2^31

    for (uint32_t val : data) {
        result *= static_cast<int64_t>(val);
        if (result > max_limit) return 2147483647;
        if (result < min_limit) return -2147483648;
    }
    return static_cast<int32_t>(result);
}

void DataProcessor::process(int client_socket) {
    char buffer[256];
    ssize_t bytesRead = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead <= 0) return;
    buffer[bytesRead] = '\0';
    std::string login(buffer);
    login.erase(login.find_last_not_of(" \n\r\t") + 1);

    if (!users.checkUser(login)) {
        std::string err = "ERR\n";
        send(client_socket, err.c_str(), err.length(), 0);
        logger.write("Ошибка аутентификации: неизвестный логин " + login, false);
        close(client_socket);
        return;
    }

    std::string salt = auth.genSalt();
    std::string saltMsg = salt + "\n";
    send(client_socket, saltMsg.c_str(), saltMsg.length(), 0);

    bytesRead = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead <= 0) { close(client_socket); return; }
    buffer[bytesRead] = '\0';
    std::string clientHash(buffer);
    clientHash.erase(clientHash.find_last_not_of(" \n\r\t") + 1);

    if (!auth.checkAuth(login, clientHash, salt, users)) {
        std::string err = "ERR\n";
        send(client_socket, err.c_str(), err.length(), 0);
        logger.write("Ошибка аутентификации: неверный пароль для " + login, false);
        close(client_socket);
        return;
    }

    std::string ok = "OK\n";
    send(client_socket, ok.c_str(), ok.length(), 0);
    logger.write("Успешная аутентификация: " + login, false);

    uint32_t vector_count = 0;
    if (recv(client_socket, &vector_count, sizeof(vector_count), 0) <= 0) {
        close(client_socket);
        return;
    }

    for (uint32_t i = 0; i < vector_count; ++i) {
        uint32_t vector_len = 0;
        if (recv(client_socket, &vector_len, sizeof(vector_len), 0) <= 0) break;

        std::vector<uint32_t> vec(vector_len);
        size_t total_received = 0;
        while (total_received < vector_len * sizeof(uint32_t)) {
            ssize_t r = recv(client_socket, reinterpret_cast<char*>(vec.data()) + total_received, 
                             vector_len * sizeof(uint32_t) - total_received, 0);
            if (r <= 0) break;
            total_received += r;
        }

        int32_t calculation_result = calcProduct(vec);
        send(client_socket, &calculation_result, sizeof(calculation_result), 0);
    }
    close(client_socket);
}
