#pragma once

#include <Core/DirectX12/DirectX12.h>
#include <Common/structs.h>
#include "ParticleSystem.h"
#include <wrl/client.h>
#include <vector>
#include <cstdint>
#include <string>
#include <d3d12.h>
#include <Matrix4x4.h>
#include <Vector3.h>
#include "./Type/ParticleType.h"
#include "./Emitter/EmitterData.h"
#include <Features/Model/IModel.h>
#include <DebugTools/DebugEntry/DebugEntry.h>
#include <memory>

/// <summary>
/// パーティクルクラス
/// </summary>
class Particle : public EngineFeature
{
public:
    Particle() = default;
    /// <summary>
    /// パーティクルを初期化します。
    /// </summary>
    /// <param name="_pModel">使用するモデル。</param>
    void Initialize(IModel* _pModel);
    /// <summary>
    /// パーティクルの描画を行います。
    /// </summary>
    void Draw();
    /// <summary>
    /// パーティクルの更新を行います。
    /// </summary>
    void Update();
    /// <summary>
    /// リソースの解放を行います。
    /// </summary>
    void Finalize();
    /// <summary>
    /// デバッグUIを描画します。
    /// </summary>
    void ImGui();


public: /// Setter
    void SetName(const std::string& name) { if (pDebugEntry_) pDebugEntry_->SetName(name); }
    void SetEnableBillboard(bool _enable) { enableBillboard_ = _enable; }
    void SetGameEye(GameEye** _pGameEye) { pGameEye_ = _pGameEye; }


public: /// Getter
    bool GetEnableBillboard() const { return enableBillboard_; }
    auto& GetParticleData() { return particleData_; }
    bool IsAbleDelete() const { return particleData_.empty(); }


public: /// container operator
    void reserve(size_t _size, bool _isInit = false);
    void emplace_back(const ParticleData& _data);


private:
    /// Common
    std::unique_ptr<DebugEntry<Particle>>   pDebugEntry_                        = nullptr;

    /// GameEye
    GameEye**                               pGameEye_                           = nullptr;

    /// Instancing
    Microsoft::WRL::ComPtr<ID3D12Resource>  instancingResource_                 = nullptr;
    ParticleForGPU*                         instancingData_                     = nullptr;
    uint32_t                                currentInstancingSize_              = 0u;

    /// Model
    std::string                             modelPath_                          = {};
    IModel*                                 pModel_                             = nullptr;
    size_t                                  vertexCount_                        = 0u;
    D3D12_VERTEX_BUFFER_VIEW                vertexBufferView_                   = {};

    /// SRV
    uint32_t                                srvIndex_                           = 0u;
    D3D12_CPU_DESCRIPTOR_HANDLE             srvCpuHandle_                       = {};
    D3D12_GPU_DESCRIPTOR_HANDLE             srvGpuHandle_                       = {};
    D3D12_GPU_DESCRIPTOR_HANDLE             textureSRVHandleGPU_                = {};

    /// Billboard
    Matrix4x4                               backToFrontMatrix_                  = {};
    Matrix4x4                               billboardMatrix_                    = {};
    bool                                    enableBillboard_                    = false;

    /// Parameter
    std::vector<ParticleData>               particleData_                       = {};


private: /// 他クラスのインスタンス
    ID3D12Device* pDevice_ = nullptr;
    ParticleSystem* pSystem_ = nullptr;


private:
    /// <summary>
    /// GPU向けインスタンシング用バッファを作成します。
    /// </summary>
    void CreateParticleForGPUResource();
    /// <summary>
    /// SRV を作成します。
    /// </summary>
    void CreateSRV();
    /// <summary>
    /// モデルの頂点情報などを取得します。
    /// </summary>
    void GetModelData();
    /// <summary>
    /// 変換情報を初期化します。
    /// </summary>
    void InitializeTransform();
    /// <summary>
    /// 1パーティクル分のデータを更新します。
    /// </summary>
    void ParticleDataUpdate(std::vector<ParticleData>::iterator& _itr);
    /// <summary>
    /// イージング関数（EaseOutCubic）。
    /// </summary>
    float EaseOutCubic(float t);
    /// <summary>
    /// イージング関数（EaseOutQuad）。
    /// </summary>
    float EaseOutQuad(float t);
    /// <summary>
    /// 床との衝突による位置/速度の更新を行います。
    /// </summary>
    bool UpdateByCollisionFloor(Vector3& _position, Vector3& _velocity, const v3::CollisionFloor& _floor, float _radius);
    /// <summary>
    /// 摩擦を適用します。
    /// </summary>
    void ApplyFriction(Vector3& _velocity, bool _isGround, float _frictionCoef, float _deltaTime);

private: /// delete condition
    /// <summary>
    /// パーティクル削除条件を判定します。
    /// </summary>
    bool ParticleDeleteByCondition(std::vector<ParticleData>::iterator& _itr);
    /// <summary>
    /// 寿命によって削除するか判定します。
    /// </summary>
    bool DeleteByLifeTime(std::vector<ParticleData>::iterator& _itr);
    /// <summary>
    /// 透明度が0になった場合に削除するか判定します。
    /// </summary>
    bool DeleteByZeroAlpha(std::vector<ParticleData>::iterator& _itr);
};