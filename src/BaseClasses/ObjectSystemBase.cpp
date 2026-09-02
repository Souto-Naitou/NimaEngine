#include "./ObjectSystemBase.h"

IGameEye** ObjectSystemBase::GetGlobalEye()
{
    return &pGlobalEye_;
}

void ObjectSystemBase::SetGlobalEye(IGameEye* pGameEye)
{
    pGlobalEye_ = pGameEye;
}

void ObjectSystemBase::SetRTVHandle(const D3D12_CPU_DESCRIPTOR_HANDLE* handle)
{
    rtvHandle_ = handle;
}