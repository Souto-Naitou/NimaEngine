#pragma once

#include <nlohmann/json.hpp>

namespace utl
{
    namespace json
    {
        void try_assign(const nlohmann::json& j, const std::string& key, auto& outval)
        {
            auto itr = j.find(key);
            if (itr == j.end()) outval = {};
            else itr->get_to(outval);
        }

        void try_assign(
            const nlohmann::json& j,
            const std::string& key,
            auto&& func,
            auto& outval
        )
        {
            auto itr = j.find(key);
            if (itr == j.end()) outval = {};
            else func(itr.value(), outval);
        }
    }
}