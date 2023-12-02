#include "resourcemanager.h"
#include <filesystem>
#include <physfs.h>
#include <fstream>
#include "string.h"
#include "luainterface.h"
#include <Windows.h>

void ResourceManager::init()
{
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
        setConfigPath(path);
}


std::string ResourceManager::guessFilePath(const std::string& filename, const std::string& type)
{
    if (isFileType(filename, type))
        return filename;
    return filename + "." + type;
}

bool ResourceManager::isFileType(const std::string& filename, const std::string& type)
{
    if (filename.ends_with(std::string(".") + type))
        return true;
    return false;
}

std::string ResourceManager::readFileContents(const std::string& fileName)
{
    std::ifstream file(fileName);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + fileName);
    }

    std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    return contents;
}

std::string ResourceManager::resolvePath(const std::string& path)
{
    std::string fullPath;
    if (path.starts_with("/"))
        fullPath = path;
    else {
        if (const std::string scriptPath = "/" + g_lua.getCurrentSourcePath(); !scriptPath.empty())
            fullPath += scriptPath + "/";
        fullPath += path;
    }
    if (!(fullPath.starts_with("/")))
        //g_logger.traceWarning(stdext::format("the following file path is not fully resolved: %s", path));

    stdext::replace_all(fullPath, "//", "/");
    return fullPath;
}

bool ResourceManager::fileExists(const std::string& fileName)
{
    //if (fileName.find("/downloads") != std::string::npos)
    //    return g_http.getFile(fileName.substr(10)) != nullptr;

    return (PHYSFS_exists(resolvePath(fileName).c_str()) && !directoryExists(fileName));
}

bool ResourceManager::directoryExists(const std::string& directoryName)
{

    PHYSFS_Stat stat = {};
    if (!PHYSFS_stat(resolvePath(directoryName).c_str(), &stat)) {
        return false;
    }

    return stat.filetype == PHYSFS_FILETYPE_DIRECTORY;
}

std::list<std::string> ResourceManager::listDirectoryFiles(const std::string& directoryPath, bool fullPath /* = false */, bool raw /*= false*/, bool recursive)
{
    std::list<std::string> files;
    const auto path = raw ? directoryPath : resolvePath(directoryPath);
    const auto rc = PHYSFS_enumerateFiles(path.c_str());

    if (!rc)
        return files;

    for (int i = 0; rc[i] != nullptr; i++) {
        std::string fileOrDir = rc[i];
        if (fullPath) {
            if (path != "/")
                fileOrDir = path + "/" + fileOrDir;
            else
                fileOrDir = path + fileOrDir;
        }

        if (recursive && directoryExists("/" + fileOrDir)) {
            const auto& moreFiles = listDirectoryFiles(fileOrDir, fullPath, raw, recursive);
            files.insert(files.end(), moreFiles.begin(), moreFiles.end());
        }
        else {
            files.push_back(fileOrDir);
        }
    }

    PHYSFS_freeList(rc);
    files.sort();
    return files;
}

std::string ResourceManager::getBaseDir()
{
    return PHYSFS_getBaseDir();
}

bool ResourceManager::discoverWorkDir(const std::string& existentFile)
{
    // search for modules directory
    std::string possiblePaths[] = {
                                    g_resources.getBaseDir(),
                                    g_resources.getBaseDir() + "/game_data/",
                                    g_resources.getBaseDir() + "../" };

    bool found = false;
    for (const auto& dir : possiblePaths) {
        if (!PHYSFS_mount(dir.c_str(), nullptr, 0))
            continue;

        if (PHYSFS_exists(existentFile.c_str())) {
            /// convert below function to exception
            ///g_logger.debug(stdext::format("Found work dir at '%s'", dir));
            configPath = dir;
            found = true;
            break;
        }
        PHYSFS_unmount(dir.c_str());
    }

    return found;
}