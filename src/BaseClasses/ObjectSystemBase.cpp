#include "./ObjectSystemBase.h"

GameEye** ObjectSystemBase::GetGlobalEye()
{
    return &pGlobalEye_;
}

void ObjectSystemBase::SetGlobalEye(GameEye* pGameEye)
{
    pGlobalEye_ = pGameEye;
}

void ObjectSystemBase::SetRTVHandle(const D3D12_CPU_DESCRIPTOR_HANDLE* handle)
{
    rtvHandle_ = handle;
}