#include "ConfigHandler.h"
#include <iostream>

ConfigHandler::ConfigHandler() 
    : configFile("./vcalc.conf"), logFile("./vcalc.log"), port(33333), desc("Allowed options") 
{
    desc.add_options()
        ("help,h", "Показать справку")
        ("config,c", po::value<std::string>(&configFile)->default_value("./vcalc.conf"), "Путь к базе клиентов")
        ("log,l", po::value<std::string>(&logFile)->default_value("./vcalc.log"), "Путь к файлу журнала")
        ("port,p", po::value<uint16_t>(&port)->default_value(33333), "TCP-порт сервера");
}

bool ConfigHandler::parse(int argc, char* argv[]) {
    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        if (vm.count("help")) { printHelp(); return false; }
        po::notify(vm);
    } catch (const std::exception& e) {
        std::cerr << "Ошибка CLI: " << e.what() << "\n";
        printHelp();
        return false;
    }
    return true;
}

void ConfigHandler::printHelp() const { std::cout << desc << "\n"; }
