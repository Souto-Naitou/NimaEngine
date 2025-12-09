#pragma once

#include <Vector3.h>
#include <Vector4.h>
#include <Range.h>

#include <nlohmann/json.hpp>

inline void from_json(const nlohmann::json& _j, Vector3& _v)
{
    _j.at("x").get_to(_v.x);
    _j.at("y").get_to(_v.y);
    _j.at("z").get_to(_v.z);
}

inline void to_json(nlohmann::json& _j, const Vector3& _v)
{
    _j = nlohmann::json{ {"x", _v.x}, {"y", _v.y}, {"z", _v.z} };
}

inline void from_json(const nlohmann::json& _j, Vector4& _v)
{
    _j.at("x").get_to(_v.x);
    _j.at("y").get_to(_v.y);
    _j.at("z").get_to(_v.z);
    _j.at("w").get_to(_v.w);
}

inline void to_json(nlohmann::json& _j, const Vector4& _v)
{
    _j = nlohmann::json{ {"x", _v.x}, {"y", _v.y}, {"z", _v.z}, {"w", _v.w} };
}

template <typename T>
inline void from_json(const nlohmann::json& _j, Range<T>& _v)
{
    _j.at("start").get_to(_v.start);
    _j.at("end").get_to(_v.end);
}

template <typename T>
inline void to_json(nlohmann::json& _j, const Range<T>& _v)
{
    _j = nlohmann::json{ {"start", _v.start}, {"end", _v.end} };
}