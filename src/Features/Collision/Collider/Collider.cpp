#include "Collider.h"
#include <DebugTools/ImGuiTemplates/ImGuiTemplates.h>
#include <sstream>
#include <Utility/Debug/dbgutl.h>
#include <DebugTools/DebugManager/DebugManager.h>

Collider::Collider(bool enableDebugWindow) : isEnableDebugWindow_(enableDebugWindow)
{
    hexID_ = utl::debug::generate_name_default(this);
    if (isEnableDebugWindow_)
    {
        pDebugEntry_ = std::make_unique<DebugEntry<Collider>>("Colliders", hexID_, this, false);
    }
}

const bool Collider::IsRegisteredCollidingPtr(const Collider* ptr) const
{
    for (auto itr = collidingPtrList_.begin(); itr != collidingPtrList_.end(); ++itr)
    {
        if (ptr == *itr) return true;
    }
    return false;
}

void Collider::EraseCollidingPtr(const Collider* ptr)
{
    collidingPtrList_.remove_if([ptr](const Collider* pCollider) {
        return pCollider == ptr;
    });
    return;
}

void Collider::SetOwnerTransform(EulerTransform* pTransform)
{
    pOwnerTransform_ = pTransform;
}

void Collider::SetColliderID(const std::string& id)
{
    colliderID_ = id;
    hexID_ = utl::debug::generate_name(id, this);
}

void Collider::SetAttribute(uint32_t attribute)
{
    attribute_ = attribute;
}

void Collider::SetMask(uint32_t* mask)
{
    pMask_ = mask;
}

void Collider::SetOnCollision(const std::function<void(const Collider*)>& func)
{
    funcOnCollision_ = func;
}

void Collider::SetOnCollisionTrigger(const std::function<void(const Collider*)>& func)
{
    funcOnCollisionTrigger_ = func;
}

void Collider::SetSphereForBroadPhase(const Sphere& sphere)
{
    sphereForBroadPhase_ = sphere;
}

void Collider::RegisterCollidingPtr(const Collider* ptr)
{
    collidingPtrList_.push_back(ptr);
}

void Collider::OnCollision(const Collider* other)
{
    if (funcOnCollision_) funcOnCollision_(other);
}

void Collider::OnCollisionTrigger(const Collider* other)
{
    if (funcOnCollisionTrigger_) funcOnCollisionTrigger_(other);
}

void Collider::ImGui()
{
#ifdef _DEBUG

    ImGui::Text("Attribute: %x", attribute_);
    auto pFunc = [&]()
    {
        for (auto ptr : collidingPtrList_)
        {
            std::stringstream ss;
            ss << "0x" << std::hex << reinterpret_cast<uintptr_t>(ptr);
            ImGuiTemplate::VariableTableRow(ss.str(), ptr->GetColliderID());
        }
    };

    ImGuiTemplate::VariableTable("Collider", pFunc);

#endif // _DEBUG
}
