#pragma once
#include <string>
#include <Math/Transform.h>
#include <Vector3.h>
#include <variant>
#include <optional>
#include <list>
#include <nlohmann/json.hpp>
#include <Vector2.h>

/// <summary>
/// Blenderレベルデータ構造体群
/// </summary>
namespace BlenderLevel
{
    void assign_from_json_vec3(const nlohmann::json& _j, Vector3& _vec);
    void assign_from_json_vec2(const nlohmann::json& _j, Vector2& _vec);

    // =============================================
    // [Transform Begin]
    using Transform = EulerTransform;
    void assign_from_json(const nlohmann::json& _j, EulerTransform& _transform);
    // [Transform End]
    // =============================================

    // =============================================
    // [Collider Begin]
    struct BoxCollider
    {
        Vector3 center;
        Vector3 size;
    };
    void from_json(const nlohmann::json& _j, BoxCollider& _boxCollider);

    struct SphereCollider
    {
        Vector3 center;
        float radius;
    };
    void from_json(const nlohmann::json& _j, SphereCollider& _sphereCollider);

    using Collider = std::variant<BoxCollider, SphereCollider>;

    void assign_from_json(const nlohmann::json& _j, Collider& _collider);
    // [Collider End]
    // =============================================

    struct Object
    {
        // e.g. "MESH", "LIGHT", "CAMERA"
        std::string type;
        std::string name;
        Transform transform;
        std::string filename;
        std::optional<Collider> collider;
        Vector2 uvScale = { 1.0f, 1.0f }; // UVスケール
    };
    void from_json(const nlohmann::json& _j, Object& _object);

    struct LevelData
    {
        std::string name;
        std::list<Object> objects;
    };

    void from_json(const nlohmann::json& j, LevelData& levelData);
}