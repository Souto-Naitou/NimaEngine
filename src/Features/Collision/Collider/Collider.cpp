#include "Collider.h"
#include <Features/Collision/Manager/CollisionManager.h>
#include <DebugTools/ImGuiTemplates/ImGuiTemplates.h>
#include <sstream>
#include <Utility/Debug/dbgutl.h>

Collider::Collider(bool enableDebugWindow) : isEnableDebugWindow_(enableDebugWindow)
{
    hexID_ = utl::debug::generate_name_default(this);
    if (isEnableDebugWindow_)
    {
        DebugManager::GetInstance()->SetComponent("Colliders", hexID_, std::bind(&Collider::ImGui, this));
    }
}

Collider::~Collider()
{
    if (isEnableDebugWindow_)
    {
        DebugManager::GetInstance()->DeleteComponent("Colliders", hexID_);
    }
}

void Collider::DrawArea()
{
    switch (shape_)
    {
    case Shape::AABB:
        std::get<AABB*>(shapeData_)->Draw();
        break;
    case Shape::OBB:
        std::get<OBB*>(shapeData_)->Draw();
        break;
    case Shape::Sphere:
        break;
    default:
        break;
    }
}

const bool Collider::IsRegisteredCollidingPtr(const Collider* _ptr) const
{
    for (auto itr = collidingPtrList_.begin(); itr != collidingPtrList_.end(); ++itr)
    {
        if (_ptr == *itr) return true;
    }
    return false;
}

void Collider::EraseCollidingPtr(const Collider* _ptr)
{
    collidingPtrList_.remove_if([_ptr](const Collider* _pCollider) {
        return _pCollider == _ptr;
    });
    return;
}

void Collider::SetColliderID(const std::string& id)
{
    colliderID_ = id;
    hexID_ = utl::debug::generate_name(id, this);
}

void Collider::SetAttribute(uint32_t attribute)
{
    collisionAttribute_ = attribute;
}

void Collider::SetMask(uint32_t* mask)
{
    pCollisionMask_ = mask;
}

void Collider::OnCollisionTrigger(const Collider* other)
{
    if (onCollisionTriggerFunction_)
        onCollisionTriggerFunction_(other);
    return;
}

void Collider::ImGui()
{
#ifdef _DEBUG

    ImGui::Text("Attribute: %x", collisionAttribute_);
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
