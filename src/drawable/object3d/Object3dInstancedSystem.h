#pragma once
#include <BaseClasses/ObjectSystemBase.h>
#include <Core/DirectX12/PipelineStateObject/PSOCache.h>
#include <Features/Model/IModel.h>
#include <Features/Lighting/DirectionalLight.h>
#include <Features/Lighting/PointLight.h>
#include <Features/GameEye/GameEye.h>

class Object3dInstancedSystem : public ObjectSystemBase
{
public:
    struct CommandListData
    {
        std::map<UINT, ID3D12Resource*> cbuffers;
        D3D12_GPU_DESCRIPTOR_HANDLE instanceDataHandle = {};
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = {};
        IModel* model = nullptr;
    };

    Object3dInstancedSystem() = default;
    Object3dInstancedSystem(const Object3dInstancedSystem&) = delete;
    Object3dInstancedSystem& operator=(const Object3dInstancedSystem&) = delete;
    Object3dInstancedSystem(Object3dInstancedSystem&&) = delete;
    Object3dInstancedSystem& operator=(Object3dInstancedSystem&&) = delete;

    static Object3dInstancedSystem* GetInstance()
    {
        static Object3dInstancedSystem instance;
        return &instance;
    }

    void Initialize();
    void DrawSingle(ID3D12GraphicsCommandList* cl, CommandListData& data, uint32_t numInstance);

    /// [ Setters ]
    void SetEnvironmentTexture(D3D12_GPU_DESCRIPTOR_HANDLE handle) { envTexSrvHandleGpu_ = handle; }
    void SetDefaultLight(DirectionalLight* pDirectionalLight) { pDirectionalLight_ = pDirectionalLight; }
    void SetDefaultLight(PointLight* pPointLight) { pPointLight_ = pPointLight; }

    /// [ Getters ]
    DirectionalLight* GetDirectionalLight() const { return pDirectionalLight_; }
    PointLight* GetPointLight() const { return pPointLight_; }

private:
    void RegisterRootSignature();
    void RegisterPipelineState();

    static constexpr uint32_t       kRootParameterIndexEnvTexture_          = 7u;

    std::string                     name_               = "Object3dInstanced";
    PSOID                           psoId_              = "Object3dInstanced";
    RootSignatureID                 rootSignatureId_    = "Object3dInstanced";
    std::wstring                    vertexShaderPath_   = L"EngineResources/Shaders/InstancedObject3d.VS.hlsl";
    std::wstring                    pixelShaderPath_    = L"EngineResources/Shaders/Object3d.PS.hlsl";
    D3D12_GPU_DESCRIPTOR_HANDLE     envTexSrvHandleGpu_ = {}; // Environment texture SRV handle

    ID3D12PipelineState*            pPSO_               = nullptr;
    ID3D12RootSignature*            pRootSignature_     = nullptr;

    DirectionalLight*               pDirectionalLight_  = nullptr;    // デフォルトの平行光源
    PointLight*                     pPointLight_        = nullptr;    // デフォルトの点光源
};