#pragma once

#include <Features/Collision/Shape.h>
#include <Features/Primitive/AABB.h>
#include <Features/Primitive/OBB.h>
#include "Vector2.h"
#include <vector>
#include <string>
#include <functional>
#include <list>
#include <DebugTools/DebugManager/DebugManager.h>
#include <variant>


class CollisionManager;

/// <summary>
/// コライダークラス
/// </summary>
class Collider
{
public:

    Collider(bool _enableDebugWindow = true);
    ~Collider();

    void DrawArea();

public: /// Getter
    template <typename T>
    inline  const T*                    GetOwner()                  const       { return static_cast<T*>(owner_); }

    inline  const AABB*                 GetAABB()                   const       { return std::get<AABB*>(shapeData_); }
    inline  AABB*                       GetAABB()                               { return std::get<AABB*>(shapeData_); }
    inline  const OBB*                  GetOBB()                    const       { return std::get<OBB*>(shapeData_); }
    inline  OBB*                        GetOBB()                                { return std::get<OBB*>(shapeData_); }
    inline  const Sphere*               GetSphere()                 const       { return std::get<Sphere*>(shapeData_); }

    inline  uint32_t                    GetCollisionAttribute()     const       { return collisionAttribute_; }
    inline  uint32_t                    GetCollisionMask()          const       { return pCollisionMask_ ? *pCollisionMask_ : 0xffffffff; }
    inline  Shape                       GetShape()                  const       { return shape_; }
    inline  unsigned int                GetRadius()                 const       { return radiusCollider_; }
    inline  const std::string&          GetColliderID()             const       { return colliderID_; }
    inline  bool                        GetIsEnableLighter()        const       { return enableLighter_; }
    inline  Vector3                     GetPosition()               const       { return position_; }
    inline  const bool                  GetEnable()                 const       { return isEnableCollision_; }

            const bool                  IsRegisteredCollidingPtr(const Collider* _ptr) const;
            void                        EraseCollidingPtr(const Collider* _ptr);


public: /// Setter
    inline  void                        SetOwner(void* owner)                  { owner_ = owner; }
    void                                SetColliderID(const std::string& id);

    template<typename T>
    void                                SetShapeData(T* _shapeData) { shapeData_ = _shapeData; }
    void                                SetShape(Shape shape) { shape_ = shape; }

    void                                SetAttribute(uint32_t attribute);
    void                                SetMask(uint32_t* mask);
    void                                SetOnCollision(const std::function<void(const Collider*)>& _func) { onCollisionFunction_ = _func; }
    void                                SetOnCollisionTrigger(const std::function<void(const Collider*)>& _func) { onCollisionTriggerFunction_ = _func; }
    void                                SetRadius(unsigned int _rad) { radiusCollider_ = _rad; }
    void                                SetPosition(const Vector3& _v) { position_ = _v; }
    void                                SetEnableLighter(bool _flag) { enableLighter_ = _flag; }
    void                                SetEnable(bool _flag) { isEnableCollision_ = _flag; }
    void                                RegisterCollidingPtr(const Collider* _ptr) { collidingPtrList_.push_back(_ptr); }


    inline  void                        OnCollision(const Collider* other)
    {
        if (onCollisionFunction_)
            onCollisionFunction_(other);
        return;
    }

    void OnCollisionTrigger(const Collider* other);

private:

    std::function<void(const Collider*)>    onCollisionFunction_;
    std::function<void(const Collider*)>    onCollisionTriggerFunction_;
    std::variant<OBB*, AABB*, Sphere*>      shapeData_          = {};

    bool                            isEnableDebugWindow_        = true;                         // デバッグウィンドウを表示するかどうか
    void*                           owner_                      = nullptr;
    bool                            isEnableCollision_          = true;                         // 判定をするかどうか
    Shape                           shape_                      = Shape::Sphere;                // 形状
    std::string                     colliderID_                 = {};                           // ID
    std::string                     hexID_                      = {};                           // HexID


    std::list<const Collider*>      collidingPtrList_           = {};                           // 現在あたっているコライダーのリスト

    /// 軽量化用
    unsigned int                    radiusCollider_             = 0u;
    Vector3                         position_                   = {};
    bool                            enableLighter_              = false;

    // 衝突属性(自分)
    uint32_t                        collisionAttribute_         = 0xffffffff;
    // 衝突マスク(相手)
    uint32_t*                       pCollisionMask_             = nullptr;

private:
    void ImGui();

};