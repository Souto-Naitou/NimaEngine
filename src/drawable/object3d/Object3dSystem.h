#pragma once

#include <Common/structs.h>
#include <Vector4.h>
#include <Vector3.h>
#include <Matrix4x4.h>
#include <d3d12.h> // ID3D12Resource
#include <map> // std::map
#include <list> // std::list

#include <BaseClasses/ObjectSystemBase.h>
#include <Features/Model/IModel.h>
#include <Core/DirectX12/RootSignature/RootParameters.h>
#include <Features/Lighting/DirectionalLight.h>
#include <memory>
#include <Features/Lighting/PointLight.h>

struct IDxcBlob;

// 3D object common 
class Object3dSystem : public ObjectSystemBase
{
public:
    struct CommandListData
    {
        std::map<uint32_t, ID3D12Resource*> cbuffers; 
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = {};
        IModel* model = nullptr;
    };

public:
    // For singleton instance
    Object3dSystem(const Object3dSystem&) = delete;
    Object3dSystem(Object3dSystem&&) = delete;
    Object3dSystem& operator=(const Object3dSystem&) = delete;
    Object3dSystem& operator=(Object3dSystem&&) = delete;

    static Object3dSystem* GetInstance()
    {
        static Object3dSystem instance;
        return &instance;
    }

    /// <summary>
    /// システムの初期化を行います。
    /// </summary>
    void    Initialize();

    /// <summary>
    /// デプスプリパス用の描画設定を行います。
    /// </summary>
    void    DepthDrawSetting(ID3D12GraphicsCommandList* cl);

    /// <summary>
    /// メイン描画用の設定を行います。
    /// </summary>
    void    MainDrawSetting(ID3D12GraphicsCommandList* cl);

    /// <summary>
    /// 環境マップの SRV ハンドルを設定します。
    /// </summary>
    /// <param name="handle">GPU ディスクリプタハンドル。</param>
    void    SetEnvironmentTexture(D3D12_GPU_DESCRIPTOR_HANDLE handle);

    /// <summary>
    /// デフォルトのポイントライトを設定します。
    /// </summary>
    /// <param name="pl">設定するポイントライトへのポインタ。</param>
    void    SetPointLight(PointLight* pl) { pPointLight_ = pl; }

    /// <summary>
    /// デフォルトの平行光源を設定します。
    /// </summary>
    /// <param name="dl">設定する平行光源へのポインター。</param>
    void    SetDirectionalLight(DirectionalLight* dl) { pDirectionalLight_ = dl; }

    void    DrawSingle(ID3D12GraphicsCommandList* commandList, Object3dSystem::CommandListData& data);

    /// [ Getter ]
    const RootParameters&       GetRootParameters() const {return rootParameters_;}
    D3D12_GPU_DESCRIPTOR_HANDLE GetEnvironmentTextureSrvHandleGpu() const { return environmentTextureSrvHandleGpu_; }
    DirectionalLight*           GetDirectionalLight() const { return pDirectionalLight_; }
    PointLight*                 GetPointLight() const { return pPointLight_; }

private:
    // Ctor
    Object3dSystem();

    // Internal functions
    /// <summary>
    /// ルートシグネチャを作成します。
    /// </summary>
    void    CreateRootSignature();
    /// <summary>
    /// メイン描画用の PSO を作成します。
    /// </summary>
    void    CreateMainPipelineState(IDxcBlob* pBlobVS, IDxcBlob* pBlobPS);
    /// <summary>
    /// デプスプリパス用の PSO を作成します。
    /// </summary>
    void    CreateDepthPipelineState(IDxcBlob* pBlobVS);

    // DirectX objects and paths
    static constexpr wchar_t        kVertexShaderPath[]                     = L"EngineResources/Shaders/Object3d.VS.hlsl";
    static constexpr wchar_t        kPixelShaderPath[]                      = L"EngineResources/Shaders/Object3d.PS.hlsl";
    static constexpr const char*    kRootSignatureId_                       = "Object3dSystem";
    static constexpr uint32_t       kRootParameterIndexEnvTexture_          = 8u;
    DirectionalLight*               pDirectionalLight_                      = nullptr;  // Directional light
    PointLight*                     pPointLight_                            = nullptr;  // Point light
    ID3D12RootSignature*            rootSignature_                          = nullptr;  // Root signature
    ComPtr<ID3D12PipelineState>     psoMain_                                = nullptr;  // Pipeline state object for main drawing
    ComPtr<ID3D12PipelineState>     psoEarlyZ_                              = nullptr;  // Pipeline state object for Early-Z
    RootParameters                  rootParameters_                         = {};       // Root parameters for root signature
    D3D12_INPUT_ELEMENT_DESC        inputElementDescs_[3]                   = {};
    D3D12_INPUT_LAYOUT_DESC         inputLayoutDesc_                        = {};
    D3D12_RASTERIZER_DESC           rasterizerDesc_                         = {};
    D3D12_GPU_DESCRIPTOR_HANDLE     environmentTextureSrvHandleGpu_         = {}; // Environment texture SRV handle
};