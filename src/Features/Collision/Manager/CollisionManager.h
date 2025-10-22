#pragma once

#include <Features/Collision/Collider/Collider.h>
#include <vector>
#include <string>
#include <utility>
#include <Features/Collision/Shape.h>
#include <Features/Primitive/AABB.h>
#include <Features/Primitive/OBB.h>

/// <summary>
/// 衝突判定実行クラス   
/// </summary>
class CollisionManager
{
public:
    CollisionManager(const CollisionManager&) = delete;
    CollisionManager(const CollisionManager&&) = delete;
    CollisionManager& operator=(const CollisionManager&) = delete;
    CollisionManager& operator=(const CollisionManager&&) = delete;

    static CollisionManager* GetInstance() { static CollisionManager instance; return &instance; }

    void Initialize();
    void Finalize();
    void CheckAllCollision();
    void RegisterCollider(Collider* _collider);
    void DeleteCollider(Collider* _collider);
    void ClearCollider();
    uint32_t GetNewAttribute(std::string _id);

    template <typename... Args>
    uint32_t* GetNewMask(std::string _id, Args... _ignoreNames)
    {
        uint32_t result = 0;
        for (auto& attributePair : attributeList_)
        {
            if (_id.compare(attributePair.first) == 0)
            {
                /// 自分自身の属性は除外
                result = ~attributePair.second;
                break;
            }
        }
        for (std::string name : std::initializer_list<std::string>{ _ignoreNames... })
        {
            result ^= GetNewAttribute(name);
        }

        if (!result) result = ~result;

        uint32_t* resultPtr = nullptr;
        bool isCompare = false;
        /// 配列の中身を変更
        for (auto& maskPair : maskList_)
        {
            if (maskPair.first.compare(_id) == 0)
            {
                maskPair.second = result;
                resultPtr = &maskPair.second;
                isCompare = true;
            }
        }
        if (!isCompare) // マスクリストに登録されていなかったら
        {
            maskList_.push_back({ _id, result });
            resultPtr = &maskList_.back().second;
        }

        return resultPtr;
    }


private:
    CollisionManager() = default;

    std::vector<Collider*> colliders_;
    std::vector<std::pair<std::string, std::string>> collisionNames_;
    std::vector<std::pair<std::string, uint32_t>> attributeList_;
    std::list<std::pair<std::string, uint32_t>> maskList_;

    void ImGui();
    void CheckCollisionPair(Collider* _colA, Collider* _colB);
    void ProjectShapeOnAxis(const std::vector<Vector3>* _v, const Vector3& _axis, float& _min, float& _max);

    bool IsCollision(const AABB* _aabb1, const AABB* _aabb2);
    bool IsCollision(const AABB& _aabb, const Sphere& _sphere);

    bool IsCollision(const Sphere* _sphere1, const Sphere* _sphere2);
    bool IsCollision(const OBB* _obb1, const OBB* _obb2);

    bool IsCollision(const OBB& _obb, const Sphere& _sphere);

    float ProjectOntoAxis(const OBB* _obb, const Vector3& axis);
    bool OverlapOnAxis(const OBB* _obb1, const OBB* _obb2, const Vector3& axis);

    uint32_t countCheckCollision_ = 0ui32;
    uint32_t countWithoutFilter_ = 0ui32;
    uint32_t countWithoutLighter = 0ui32;

    std::string name_ = "CollisionManager";
};