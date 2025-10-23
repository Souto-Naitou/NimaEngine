#pragma once

#include <wrl.h>
#include <Core/DirectX12/DirectX12.h>
#include <Features/GameEye/GameEye.h>
#include <BaseClasses/ObjectSystemBaseMT.h>
#include <list>

/// <summary>
/// パーティクル描画共通
/// </summary>
class ParticleSystem : public ObjectSystemBaseMT
{
public:
    struct CommandListData
    {
        D3D12_VERTEX_BUFFER_VIEW* pVBV = nullptr;
        D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = {};
        D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandle = {};
        UINT vertexCount = 0;
        UINT instanceCount = 0;
    };

public:
    ParticleSystem(const ParticleSystem&) = delete;
    ParticleSystem& operator=(const ParticleSystem&) = delete;
    ParticleSystem(const ParticleSystem&&) = delete;
    ParticleSystem& operator=(const ParticleSystem&&) = delete;

    static ParticleSystem* GetInstance()
    {
        static ParticleSystem instance;
        return &instance;
    }

    /// <summary>
    /// パーティクル共通描画の初期化を行います。
    /// </summary>
    void Initialize() override;
    /// <summary>
    /// フレーム終端での描画（リングバッファ切替など）を行います。
    /// </summary>
    void PresentDraw();

    /// <summary>
    /// キューされた描画コマンドを発行します。
    /// </summary>
    void DrawCall();
    /// <summary>
    /// マルチスレッド環境での同期を行います。
    /// </summary>
    void Sync();


public: /// Setter
    void SetGlobalEye(GameEye* _pGameEye) { pGlobalEye_ = _pGameEye; }
    void AddCommandListData(CommandListData& _pData) { commandListDatas_.emplace_back(_pData); }

public: /// Getter
    GameEye** GetGlobalEye() { return &pGlobalEye_; }


public: /// 公開定数
    static constexpr uint32_t kMaxInstance_ = 256u;


private: /// メンバ変数
    static constexpr wchar_t kVertexShaderPath[] = L"EngineResources/Shaders/Particle.VS.hlsl";
    static constexpr wchar_t kPixelShaderPath[] = L"EngineResources/Shaders/Particle.PS.hlsl";
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_ = nullptr;

    std::list<CommandListData> commandListDatas_;


private: /// 非公開関数
    ParticleSystem() = default;
    /// <summary>
    /// ルートシグネチャを作成します。
    /// </summary>
    void CreateRootSignature();
    /// <summary>
    /// パーティクル描画用パイプラインを作成します。
    /// </summary>
    void CreatePipelineState();
};