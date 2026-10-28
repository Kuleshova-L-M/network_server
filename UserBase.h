#pragma once
#include <string>
#include <map>

class UserBase {
private:
    std::map<std::string, std::string> users;
public:
    void load(const std::string& file);
    bool checkUser(const std::string& login) const;
    std::string getPass(const std::string& login) const;
};




