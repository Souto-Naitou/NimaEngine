#pragma once

#include <Features/Collision/Shape.h>
#include <Features/Primitive/AABB.h>
#include <Features/Primitive/OBB.h>
#include <Features/Primitive/Sphere.h>
#include <DebugTools/DebugEntry/DebugEntry.h>

#include <string>
#include <functional>
#include <list>
#include <variant>


class CollisionManager;

/// <summary>
/// コライダークラス
/// </summary>
class Collider
{
public:

    Collider(bool _enableDebugWindow = true);
    ~Collider() = default;

    void ImGui();

    template <typename T>
    inline  const T*                    GetOwner()                  const       { return static_cast<T*>(owner_); }

    template <typename SHAPE>
    inline  const SHAPE*                GetShapeData()              const       { return std::get<SHAPE*>(shapeData_); }
    template <typename SHAPE>
    inline  SHAPE*                      GetShapeData()                          { return std::get<SHAPE*>(shapeData_); }

    inline  uint32_t                    GetCollisionAttribute()     const       { return collisionAttribute_; }
    inline  uint32_t                    GetCollisionMask()          const       { return pCollisionMask_ ? *pCollisionMask_ : 0xffffffff; }
    inline  Shape                       GetShape()                  const       { return shape_; }
    inline  const std::string&          GetColliderID()             const       { return colliderID_; }
    inline  bool                        GetIsEnableLighter()        const       { return enableLighter_; }
    inline  const Sphere&               GetSphereForBroadPhase()    const       { return sphereForBroadPhase_; }
    inline  const bool                  GetEnable()                 const       { return isEnableCollision_; }

            const bool                  IsRegisteredCollidingPtr(const Collider* _ptr) const;
            void                        EraseCollidingPtr(const Collider* _ptr);


public: /// Setter
    inline  void                        SetOwner(void* owner)                  { owner_ = owner; }
    void                                SetColliderID(const std::string& id);

    template<typename T>
    void                                SetShapeData(T* shapeData) { shapeData_ = shapeData; }
    void                                SetShape(Shape shape) { shape_ = shape; }

    void                                SetAttribute(uint32_t attribute);
    void                                SetMask(uint32_t* mask);
    void                                SetOnCollision(const std::function<void(const Collider*)>& func) { onCollisionFunction_ = func; }
    void                                SetOnCollisionTrigger(const std::function<void(const Collider*)>& func) { onCollisionTriggerFunction_ = func; }
    void                                SetSphereForBroadPhase(const Sphere& sphere) { sphereForBroadPhase_ = sphere; }
    void                                SetEnableLighter(bool flag) { enableLighter_ = flag; }
    void                                SetEnable(bool flag) { isEnableCollision_ = flag; }
    void                                RegisterCollidingPtr(const Collider* ptr) { collidingPtrList_.push_back(ptr); }


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
    std::unique_ptr<DebugEntry<Collider>>   pDebugEntry_        = nullptr;
    std::variant<OBB*, AABB*, Sphere*>      shapeData_          = {};

    bool                            isEnableDebugWindow_        = true;                         // デバッグウィンドウを表示するかどうか
    void*                           owner_                      = nullptr;
    bool                            isEnableCollision_          = true;                         // 判定をするかどうか
    Shape                           shape_                      = Shape::Sphere;                // 形状
    std::string                     colliderID_                 = {};                           // ID
    std::string                     hexID_                      = {};                           // HexID

    std::list<const Collider*>      collidingPtrList_           = {};                           // 現在あたっているコライダーのリスト

    /// 軽量化用
    Sphere                          sphereForBroadPhase_        = {};                           // ブロードフェーズ用の球体
    bool                            enableLighter_              = false;

    // 衝突属性(自分)
    uint32_t                        collisionAttribute_         = 0xffffffff;
    // 衝突マスク(相手)
    uint32_t*                       pCollisionMask_             = nullptr;

};