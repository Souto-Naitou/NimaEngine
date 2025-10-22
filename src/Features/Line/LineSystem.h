#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <Core/DirectX12/DirectX12.h>
#include <BaseClasses/ObjectSystemBaseMT.h>

/// <summary>
/// ライン共通
/// </summary>
class LineSystem : public ObjectSystemBaseMT
{
public:
    LineSystem(const LineSystem&) = delete;
    LineSystem& operator=(const LineSystem&) = delete;
    LineSystem(LineSystem&&) = delete;
    LineSystem& operator=(LineSystem&&) = delete;

    static LineSystem* GetInstance()
    {
        static LineSystem instance;
        return &instance;
    }

    void Initialize();
    void PresentDraw();


private:
    LineSystem() = default;
    ~LineSystem() = default;


private: /// メンバ変数
    static constexpr wchar_t kVertexShaderPath[] = L"EngineResources/Shaders/Line.VS.hlsl";
    static constexpr wchar_t kPixelShaderPath[] = L"EngineResources/Shaders/Line.PS.hlsl";
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap_;
    Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource_;

    D3D12_INPUT_ELEMENT_DESC inputElementDescs_[1] = {};
    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc_ = {};
    D3D12_BLEND_DESC blendDesc_ = {};
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc_ = {};

private: /// 処理郡
    void CreateRootSignature();
    void CreatePipelineState();
    void SetInputLayout();
    void SetBlendDesc();
    void SetDSVDesc();

private: /// 他クラスのインスタンス
    ID3D12Device* device_ = nullptr;
    IDxcUtils* dxcUtils_ = nullptr;
    IDxcCompiler3* dxcCompiler_ = nullptr;
    IDxcIncludeHandler* includeHandler_ = nullptr;
};