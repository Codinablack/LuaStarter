#include "resourcemanager.h"
#include <filesystem>
#include <physfs.h>
#include "string.h"
#include "luainterface.h"


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
    const std::string fullPath = resolvePath(fileName);

    PHYSFS_File* file = PHYSFS_openRead(fullPath.c_str());
    if (!file)
        throw Exception("unable to open file '%s': %s", fullPath, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));

    const int fileSize = PHYSFS_fileLength(file);
    std::string buffer(fileSize, 0);
    PHYSFS_readBytes(file, &buffer[0], fileSize);
    PHYSFS_close(file);

#if ENABLE_ENCRYPTION == 1
    buffer = decrypt(buffer);
#endif

    return buffer;
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