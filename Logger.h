#pragma once
#include <string>
#include <fstream>

class Logger {
private:
    std::string logFilePath; // Оставили только путь к файлу
public:
    Logger(const std::string& file);
    void write(const std::string& msg, bool critical);
};
