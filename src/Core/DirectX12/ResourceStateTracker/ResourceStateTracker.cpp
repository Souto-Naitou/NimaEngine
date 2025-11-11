#include "./ResourceStateTracker.h"

void ResourceStateTracker::Initialize(
    Microsoft::WRL::ComPtr<ID3D12Resource> pResource, 
    D3D12_RESOURCE_STATES state, 
    DXGI_FORMAT format)
{
    this->pResource_ = pResource;
    this->state_ = state;
    this->format_ = format;
}

void ResourceStateTracker::Reset()
{
    state_ = D3D12_RESOURCE_STATE_COMMON;
    pResource_ = nullptr;
}

D3D12_RESOURCE_STATES ResourceStateTracker::ChangeState(ID3D12GraphicsCommandList* _commandList, D3D12_RESOURCE_STATES _newState)
{
    if (this->state_ == _newState) return this->state_;

    D3D12_RESOURCE_STATES oldState = this->state_;

    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = this->pResource_.Get();
    barrier.Transition.StateBefore = this->state_;
    barrier.Transition.StateAfter = _newState;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    _commandList->ResourceBarrier(1, &barrier);
    this->state_ = _newState;

    return oldState;
}
