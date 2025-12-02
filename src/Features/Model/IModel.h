#pragma once
#include <d3d12.h>
#include <Core/DirectX12/DirectX12.h>
#include <Features/Model/ModelData.h>

/// <summary>
/// モデルインターフェース
/// </summary>
class IModel : public EngineFeature
{
public:
    virtual ~IModel() = default;
    virtual void Initialize() = 0;
    virtual void Finalize() = 0;
    virtual void Update() = 0;
    virtual void Draw(ID3D12GraphicsCommandList* cl) = 0;
    virtual void ChangeTexture(D3D12_GPU_DESCRIPTOR_HANDLE texSrvHnd) = 0;
    virtual void CreateGPUResource() = 0;
    virtual bool IsEndLoading() const = 0;
    virtual void Clone(IModel* src) = 0;
    virtual std::unique_ptr<IModel> Cloned() = 0;
    
    virtual D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const = 0;
    virtual size_t GetVertexCount() const = 0;
    virtual D3D12_GPU_DESCRIPTOR_HANDLE GetTextureSrvHandle() const = 0;
    virtual ModelData* GetModelData() = 0;
};