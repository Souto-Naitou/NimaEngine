#pragma once
#include <string>

namespace utl
{
    namespace debug
    {
        std::string generate_name_default(void* ptr);
        std::string generate_name(const std::string& name, void* ptr);
    }
}