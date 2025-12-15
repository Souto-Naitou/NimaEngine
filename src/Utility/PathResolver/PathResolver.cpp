#include "./PathResolver.h"

#include <filesystem>

namespace fs = std::filesystem;

void PathResolver::Initialize()
{
    searchPaths_.clear();
}

void PathResolver::Finalize()
{
    searchPaths_.clear();
}

void PathResolver::AddSearchPath(const std::string& path)
{
    for (const auto& existingPath : searchPaths_)
    {
        if (existingPath == path)
        {
            return;
        }
    }
    searchPaths_.push_back(path);
}

std::string PathResolver::GetParentPath(const std::string& fileName)
{
    fs::path directoryPath = {};
    if (!fs::exists(fileName))
    {
        for (const auto& path : searchPaths_)
        {
            fs::path filePath = path / fileName;
            if (fs::exists(filePath))
            {
                directoryPath = path;
                break;
            }
        }
    }
    else
    {
        directoryPath = fs::path(fileName).parent_path().string();
    }

    return directoryPath.string();
}

std::string PathResolver::GetFilePath(const std::string& fileName)
{
    fs::path filePath = {};
    if (!fs::exists(fileName) && !fileName.empty())
    {
        for (const auto& path : searchPaths_)
        {
            fs::path tempPath = path / fileName;
            if (fs::exists(tempPath))
            {
                filePath = tempPath;
                break;
            }
        }
    }
    else
    {
        filePath = fs::path(fileName);
    }

    return filePath.string();
}