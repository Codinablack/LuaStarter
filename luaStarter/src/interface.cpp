#include "luajit/lua.hpp"
#include "luainterface.h"
#include <iostream>
#include <cstdlib>
#include <filesystem>
#include "resourcemanager.h"
#include <Windows.h>

int main() {
	std::cout << "Starting Application" << std::endl;
	g_lua.init();
	std::cout << "Lua Initialized" << std::endl;
	g_resources.init();
	std::cout << "Resource Manager Initialized" << std::endl;
	std::string appConfig = g_resources.getConfigPath() + "\\config.lua";
	std::cout << "Loading config.lua" << std::endl;
    if (!g_lua.safeRunScript(appConfig)) {
        throw std::exception("Unable to run script config.lua!");
    }
	g_lua.createRegistry();
	std::cout << "Registry Created" << std::endl;
	std::cout << "Press enter to exit" << std::endl;
	std::cin.ignore(std::cin.rdbuf()->in_avail());
	return 0;
}