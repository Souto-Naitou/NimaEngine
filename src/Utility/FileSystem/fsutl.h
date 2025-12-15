#pragma once
#include <filesystem>

namespace utl::filesystem
{
    std::filesystem::path to_lower(const std::filesystem::path& path);
    std::filesystem::path get_parent_path(const std::filesystem::path& path);
    std::string get_parent_path_string(const std::filesystem::path& path);
}