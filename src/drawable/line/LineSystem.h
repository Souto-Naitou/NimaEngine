#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <Core/DirectX12/DirectX12.h>
#include <BaseClasses/ObjectSystemBase.h>
#include <map>

/// <summary>
/// ライン共通
/// </summary>
class LineSystem : public ObjectSystemBase
{
public:
    struct CommandListData
    {
        uint32_t vertexCount = 0;
        std::map<uint32_t, ID3D12Resource*> cbuffers;
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = {};
        D3D12_VERTEX_BUFFER_VIEW vbView = {};
    };

    LineSystem(const LineSystem&) = delete;
    LineSystem& operator=(const LineSystem&) = delete;
    LineSystem(LineSystem&&) = delete;
    LineSystem& operator=(LineSystem&&) = delete;

    static LineSystem* GetInstance()
    {
        static LineSystem instance;
        return &instance;
    }

    /// <summary>
    /// ライン描画システムを初期化します。
    /// </summary>
    void Initialize();
    /// <summary>
    /// バックバッファへの最終描画処理を行います。
    /// </summary>
    void DrawSetting(ID3D12GraphicsCommandList* cl);

    void    DrawSingle(ID3D12GraphicsCommandList* commandList, CommandListData& data);

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
    /// <summary>
    /// ルートシグネチャを作成します。
    /// </summary>
    void CreateRootSignature();
    /// <summary>
    /// パイプラインステートを作成します。
    /// </summary>
    void CreatePipelineState();
    /// <summary>
    /// 入力レイアウトを設定します。
    /// </summary>
    void SetInputLayout();
    /// <summary>
    /// ブレンドステートを設定します。
    /// </summary>
    void SetBlendDesc();
    /// <summary>
    /// 深度ステンシルビューの設定を行います。
    /// </summary>
    void SetDSVDesc();

private: /// 他クラスのインスタンス
    ID3D12Device* device_ = nullptr;
    IDxcUtils* dxcUtils_ = nullptr;
    IDxcCompiler3* dxcCompiler_ = nullptr;
    IDxcIncludeHandler* includeHandler_ = nullptr;
};