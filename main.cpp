#include <iostream>
#include "ConfigHandler.h"
#include "Server.h"

int main(int argc, char* argv[]) {
    try {
        ConfigHandler config;
        if (!config.parse(argc, argv)) return 0;

        Server server(config);
        server.start();
    } catch (const std::exception& e) {
        std::cerr << "Критическая ошибка: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
