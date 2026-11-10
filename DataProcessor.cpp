#include "DataProcessor.h"
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <vector>

DataProcessor::DataProcessor(UserBase& u, Authenticator& a, Logger& l) 
    : users(u), auth(a), logger(l) {}

// ИСПРАВЛЕННАЯ ФУНКЦИЯ (безопасная от переполнения)
int32_t DataProcessor::calcProduct(const std::vector<uint32_t>& data) {
    if (data.empty()) return 1;
    
    uint64_t result = 1;
    for (uint32_t val : data) {
        result *= val;
        // Если переполнение вверх, возвращаем максимум int32
        if (result > 2147483647ULL) {
            return 2147483647; 
        } 
        // Если встретился ноль
        if (result == 0) {
            return 0;
        }
    }
    return static_cast<int32_t>(result);
}

bool recv_all(int socket, void* buffer, size_t length) {
    size_t total_received = 0;
    char* buf = static_cast<char*>(buffer);
    while (total_received < length) {
        ssize_t bytes = recv(socket, buf + total_received, length - total_received, 0);
        if (bytes <= 0) return false;
        total_received += bytes;
    }
    return true;
}

void DataProcessor::process(int client_socket) {
    char buffer[256];
    
    ssize_t bytesRead = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead <= 0) { close(client_socket); return; }
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
    send(client_socket, salt.c_str(), salt.length(), 0);

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
    if (!recv_all(client_socket, &vector_count, sizeof(vector_count))) {
        close(client_socket);
        return;
    }

    for (uint32_t i = 0; i < vector_count; ++i) {
        uint32_t vector_len = 0;
        if (!recv_all(client_socket, &vector_len, sizeof(vector_len))) break;
        
        std::vector<uint32_t> vec(vector_len);
        if (vector_len > 0) {
            if (!recv_all(client_socket, vec.data(), vector_len * sizeof(uint32_t))) break;
        }
        
        int32_t calculation_result = calcProduct(vec);
        uint32_t results_count = 1;
        send(client_socket, &results_count, sizeof(results_count), 0);
        send(client_socket, &calculation_result, sizeof(calculation_result), 0);
    }
    close(client_socket);
}
