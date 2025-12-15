#include "fsutl.h"

#include <Utility/String/strutl.h>

std::filesystem::path utl::filesystem::to_lower(const std::filesystem::path& path)
{
    std::filesystem::path result = {};

    result = string::to_lower(path.string());

    return result;
}

std::filesystem::path utl::filesystem::get_parent_path(const std::filesystem::path& path)
{
    std::filesystem::path result = {};
    if (path.has_parent_path())
    {
        result = path.parent_path();
    }
    else
    {
        result = path;
    }
    return result;
}

std::string utl::filesystem::get_parent_path_string(const std::filesystem::path& path)
{
    return get_parent_path(path).string();
}
