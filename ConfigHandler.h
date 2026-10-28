#pragma once
#include <string>
#include <cstdint>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

class ConfigHandler {
private:
    std::string configFile;
    std::string logFile;
    uint16_t port;
    po::options_description desc;
public:
    ConfigHandler();
    bool parse(int argc, char* argv[]);
    void printHelp() const;
    std::string getConfigFile() const { return configFile; }
    std::string getLogFile() const { return logFile; }
    uint16_t getPort() const { return port; }
};
