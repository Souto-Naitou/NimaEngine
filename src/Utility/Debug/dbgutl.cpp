#include "dbgutl.h"
#include <Utility/String/strutl.h>

std::string utl::debug::generate_name_default(void* ptr)
{
    std::string result = "unnamed##";
    result += utl::string::to_string(ptr);
    return result;
}

std::string utl::debug::generate_name(const std::string& name, void* ptr)
{
    std::string result = name + "##";
    result += utl::string::to_string(ptr);
    return result;
}
