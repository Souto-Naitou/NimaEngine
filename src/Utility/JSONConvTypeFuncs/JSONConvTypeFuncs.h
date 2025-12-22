#pragma once

#include <Vector3.h>
#include <Vector4.h>
#include <Range.h>

#include <nlohmann/json.hpp>

inline void from_json(const nlohmann::json& j, Vector3& v)
{
    j.at("x").get_to(v.x);
    j.at("y").get_to(v.y);
    j.at("z").get_to(v.z);
}

inline void to_json(nlohmann::json& j, const Vector3& v)
{
    j = nlohmann::json{ {"x", v.x}, {"y", v.y}, {"z", v.z} };
}

inline void from_json(const nlohmann::json& j, Vector4& v)
{
    j.at("x").get_to(v.x);
    j.at("y").get_to(v.y);
    j.at("z").get_to(v.z);
    j.at("w").get_to(v.w);
}

inline void to_json(nlohmann::json& j, const Vector4& v)
{
    j = nlohmann::json{ {"x", v.x}, {"y", v.y}, {"z", v.z}, {"w", v.w} };
}

template <typename T>
inline void from_json(const nlohmann::json& j, Range<T>& v)
{
    j.at("start").get_to(v.start);
    j.at("end").get_to(v.end);
}

template <typename T>
inline void to_json(nlohmann::json& j, const Range<T>& v)
{
    j = nlohmann::json{ {"start", v.start}, {"end", v.end} };
}