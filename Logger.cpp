#include "Logger.h"
#include <chrono>
#include <iomanip>

Logger::Logger(const std::string& file) : logFilePath(file) {}

void Logger::write(const std::string& msg, bool critical) {
    std::lock_guard<std::mutex> lock(mtx);
    std::ofstream file(logFilePath, std::ios::app);
    if (!file.is_open()) return;
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    file << "[" << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S") << "] "
         << (critical ? "[CRITICAL] " : "[INFO] ") << msg << "\n";
}
