#include "UserBase.h"
#include "ServerException.h"
#include <fstream>

void UserBase::load(const std::string& file) {
    std::ifstream ifs(file);
    if (!ifs.is_open()) {
        throw ServerException("Не удалось открыть базу пользователей: " + file, true);
    }
    std::string line;
    while (std::getline(ifs, line)) {
        if (line.empty()) continue;
        size_t pos = line.find(':');
        if (pos != std::string::npos) {
            users[line.substr(0, pos)] = line.substr(pos + 1);
        }
    }
}

bool UserBase::checkUser(const std::string& login) const {
    return users.find(login) != users.end();
}

std::string UserBase::getPass(const std::string& login) const {
    auto it = users.find(login);
    return (it != users.end()) ? it->second : "";
}
