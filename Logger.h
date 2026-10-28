#pragma once
#include <string>
#include <fstream>
#include <mutex>

class Logger {
private:
    std::string logFilePath;
    std::mutex mtx;

public:
    Logger(const std::string& file);
    void write(const std::string& msg, bool critical);
};
