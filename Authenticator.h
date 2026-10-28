#pragma once
#include <string>
#include "UserBase.h"

class Authenticator {
public:
    std::string genSalt() const;
    std::string calcHash(const std::string& salt, const std::string& pass) const;
    bool checkAuth(const std::string& login, const std::string& clientHash, const std::string& salt, const UserBase& users) const;
};
