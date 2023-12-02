#include "luajit/lua.hpp"
#include "luainterface.h"
#include <iostream>
#include <cstdlib>
#include <filesystem>
#include "resourcemanager.h"
#include <Windows.h>

std::string get_executable_path() {
    std::string path = "./";
#ifdef _WIN32
    // Get the path of the currently running executable
    char buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH);
    // Get the directory of the executable
    path = std::filesystem::path(buffer).parent_path().string();
#elif __linux__
    // Get the path of the currently running executable
    char** argv = NULL;
    int argc = 0;
    char* exec_path = realpath(argv[0], NULL);
    path = std::filesystem::path(exec_path).parent_path().string();
    free(exec_path);
#elif __APPLE__
    // Get the path of the currently running executable
    char buffer[PATH_MAX];
    uint32_t bufsize = sizeof(buffer);
    _NSGetExecutablePath(buffer, &bufsize);
    // Get the directory of the executable
    path = std::filesystem::path(buffer).parent_path().string();
#endif
    return path;
}

int main() {
	std::cout << "Starting interface" << std::endl;
	g_lua.init();
	std::cout << get_executable_path() << std::endl;
	//g_resources.discoverWorkDir("init.lua");
	std::string appConfig = get_executable_path() + "\\config.lua";
	//g_resources.loadFileContents(appConfig, "config.lua");
    if (!g_lua.safeRunScript(appConfig)) {
        throw std::exception("Unable to run script config.lua!");
    }
	g_lua.createRegistry();
	std::cin.ignore(std::cin.rdbuf()->in_avail());
	return 0;
}