#pragma once

#include <string>
#include <cstdio>
#include <cstdint>

std::string operator"" _s(const char* str, size_t len);

namespace utl::string
{
    template <typename T>
    inline std::string to_string(const T* ptr)
    {
        char buffer[20];
        std::snprintf(buffer, sizeof(buffer), "%p", static_cast<const void*>(ptr));
        return std::string(buffer);
    }

    uint32_t to_hash(const std::string& str);

    std::string to_lower(const std::string& str);
}