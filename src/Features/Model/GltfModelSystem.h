#pragma once

#include <Core/DirectX12/DirectX12.h>
#include <d3d12.h>
#include <wrl/client.h>
#include <dxcapi.h>

/// <summary>
/// GLTFモデル共通
/// </summary>
class GltfModelSystem : public EngineFeature
{
public:
    void Initialize();
    void PrepareDispatch();

private:
    template <typename T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;

    static constexpr wchar_t    kComputeShaderPath[]    = L"EngineResources/Shaders/Skinning.CS.hlsl";

    ComPtr<ID3D12RootSignature> rootSignature_          = nullptr;  // Root signature for compute shader
    ComPtr<ID3D12PipelineState> pso_                    = nullptr;  // Pipeline state object
    ComPtr<IDxcBlob>            computeShaderBlob_      = nullptr;  // Blob of compute shader

    void    _CompileShader();
    void    _CreatePipelineStateCS();
    void    _CreateRootSignatureCS();
};