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
#include <Interfaces/IEntityStats.h>


class CollisionManager;

/// <summary>
/// コライダークラス
/// </summary>
class Collider
{
public:

    Collider(bool _enableDebugWindow = true);
    ~Collider() = default;

    void    EraseCollidingPtr(const Collider* _ptr);
    void    OnCollision(const Collider* other);
    void    OnCollisionTrigger(const Collider* other);
    void    ImGui();

    /// [ Getter ]
    inline  const   EulerTransform*     GetOwnerTransform()         const       { return pOwnerTransform_; }
    inline  const   IEntityStats*       GetEntityStats()            const       { return pEntityStats_; }
    inline          uint32_t            GetCollisionAttribute()     const       { return attribute_; }
    inline          uint32_t            GetCollisionMask()          const       { return pMask_ ? *pMask_ : 0xffffffff; }
    inline          Shape               GetShape()                  const       { return shape_; }
    inline  const   std::string&        GetColliderID()             const       { return colliderID_; }
    inline  const   Sphere*             GetSphereForBroadPhase()    const       { return sphereForBroadPhase_.has_value() ? &sphereForBroadPhase_.value() : nullptr; }
    inline  const   bool                GetEnable()                 const       { return isEnableCollision_; }
    template <typename T>
    inline  const   T*                  GetOwner()                  const       { return static_cast<T*>(owner_); }
    template <typename SHAPE>
    inline  const   SHAPE*              GetShapeData()              const       { return std::get<SHAPE*>(shapeData_); }
    template <typename SHAPE>
    inline          SHAPE*              GetShapeData()                          { return std::get<SHAPE*>(shapeData_); }
    inline  const   bool                IsEnableTwoStepJudge()      const       { return sphereForBroadPhase_.has_value(); }
            const   bool                IsRegisteredCollidingPtr(const Collider* _ptr) const;

    /// [ Setter ]
    template<typename T>
                    void                SetShapeData(T* shapeData)              { shapeData_ = shapeData; }
    inline          void                SetOwner(void* owner)                   { owner_ = owner; }
    inline          void                SetShape(Shape shape)                   { shape_ = shape; }
    inline          void                SetEnable(bool flag)                    { isEnableCollision_ = flag; }
    inline          void                SetEntityStats(IEntityStats* pStats)    { pEntityStats_ = pStats; }
                    void                SetOwnerTransform(EulerTransform* pTransform);
                    void                SetColliderID(const std::string& id);
                    void                SetAttribute(uint32_t attribute);
                    void                SetMask(uint32_t* mask);
                    void                SetOnCollision(const std::function<void(const Collider*)>& func);
                    void                SetOnCollisionTrigger(const std::function<void(const Collider*)>& func);
                    void                SetSphereForBroadPhase(const Sphere& sphere);
                    void                RegisterCollidingPtr(const Collider* ptr);


private:
    std::function<void(const Collider*)>    funcOnCollision_;
    std::function<void(const Collider*)>    funcOnCollisionTrigger_;
    std::unique_ptr<DebugEntry<Collider>>   pDebugEntry_            = nullptr;
    std::variant<OBB*, AABB*, Sphere*>      shapeData_              = {};
    EulerTransform*                         pOwnerTransform_        = nullptr;
    IEntityStats*                           pEntityStats_           = nullptr;
    bool                                    isEnableDebugWindow_    = true;             // デバッグウィンドウを表示するかどうか
    void*                                   owner_                  = nullptr;
    bool                                    isEnableCollision_      = true;             // 判定をするかどうか
    Shape                                   shape_                  = Shape::Sphere;    // 形状
    std::string                             colliderID_             = {};               // ID
    std::string                             hexID_                  = {};               // HexID
    std::list<const Collider*>              collidingPtrList_       = {};               // 現在あたっているコライダーのリスト
    uint32_t                                attribute_              = 0xffffffff;       // 衝突属性(自分)
    uint32_t*                               pMask_                  = nullptr;          // 衝突マスク(相手)
    /// 軽量化用
    std::optional<Sphere>                   sphereForBroadPhase_    = std::nullopt;     // ブロードフェーズ用の球体

};