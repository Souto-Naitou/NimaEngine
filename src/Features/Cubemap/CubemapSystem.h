#pragma once

#include <Core/DirectX12/DirectX12.h>
#include <Core/DirectX12/PipelineStateObject/PipelineStateObject.h>
#include <Features/GameEye/GameEye.h>

/// <summary>
/// キューブマップ共通
/// </summary>
class CubemapSystem : public EngineFeature
{
public:
    CubemapSystem() = default;
    ~CubemapSystem() override = default;

    void Initialize();
    void Finalize();

    void DrawSetting();

    GameEye** GetGlobalEye() { return &pGlobalEye_; }
    void SetGlobalEye(GameEye* _pGameEye) { pGlobalEye_ = _pGameEye; }

private:
    void _CreatePSO();
    void _CreateRootSignature();

    static constexpr wchar_t kVertexShaderPath[] = L"EngineResources/Shaders/Skybox.VS.hlsl";
    static constexpr wchar_t kPixelShaderPath[] = L"EngineResources/Shaders/Skybox.PS.hlsl";

    PipelineStateObject pso_;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;
    Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob_ = nullptr;
    Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob_ = nullptr;

    GameEye* pGlobalEye_ = nullptr;  //< ゲームカメラ

    // Pointers
    ID3D12Device* device_ = nullptr;
};