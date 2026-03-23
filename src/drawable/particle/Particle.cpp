#include "Particle.h"
#include <Core/DirectX12/Helper/DX12Helper.h>
#include <Core/DirectX12/SRVManager.h>
#include <numbers>

#if defined _DEBUG
#include <imgui.h>
#include <DebugTools/ImGuiTemplates/ImGuiTemplates.h>
#endif
#include <Math/Easing.h>
#include <Math/Functions.hpp>
#include <mathExtension.h>

using namespace Type::ParticleEmitter;

void Particle::Initialize(IModel* pModel)
{
#if defined _DEBUG
    pDebugEntry_ = std::make_unique<DebugEntry<Particle>>("Particle", "unnamed", this, true);
#endif

    /// 必要なインスタンスを取得
    pDevice_ = pDx12_->GetDevice();
    pSystem_ = ParticleSystem::GetInstance();
    pRandomGenerator_ = RandomGenerator::GetInstance();

    /// デフォルトのGameEyeを取得
    pGameEye_ = pSystem_->GetGlobalEye();

    if (!currentInstancingSize_) reserve(1, true);

    /// モデルを読み込む
    pModel_ = pModel;
    if (pModel_->IsEndLoading()) this->GetModelData();

    /// 正面を向く行列を作成
    backToFrontMatrix_ = Matrix4x4::RotateYMatrix(std::numbers::pi_v<float>);
}

void Particle::Update()
{
    /// モデル情報の取得
    if (pModel_->IsEndLoading()) this->GetModelData();

    /// パーティクルの更新
    if (particleData_.empty()) return;

    uint32_t index = 0;
    auto itr = particleData_.begin();
    while (true)
    {
        if (itr == particleData_.end()) break;
        if (index >= currentInstancingSize_) break;

        EulerTransform& transform = itr->transform;
        Vector4& currentColor = itr->currentColor;

        /// パーティクルデータの更新
        ParticleDataUpdate(itr);

        /// パーティクルの条件付き削除
        if (DeleteParticleByCondition(itr))
        {
            /// 削除されたら
            if (itr == particleData_.end()) break;
            else continue;
        }

        Matrix4x4 wMatrix = {};
        Matrix4x4 scaleMatrix = Matrix4x4::ScaleMatrix(transform.scale);
        Matrix4x4 translateMatrix = Matrix4x4::TranslateMatrix(transform.translate);

        if (enableBillboard_) wMatrix = scaleMatrix * billboardMatrix_ * translateMatrix;
        else wMatrix = Matrix4x4::AffineMatrix(transform.scale, transform.rotate, transform.translate);

        instancingData_[index].world = wMatrix;
        instancingData_[index].wvp = wMatrix * (*pGameEye_)->GetViewProjectionMatrix();
        instancingData_[index].color = currentColor;

        ++itr;
        ++index;
    }

    /// ビルボード
    if (enableBillboard_)
    {
        billboardMatrix_ = backToFrontMatrix_ * (*pGameEye_)->GetWorldMatrix();
        /// 平行移動成分を除去
        for (uint32_t i = 0; i < 3; i++) billboardMatrix_.m[3][i] = 0.0f;
    }
    else
    {
        billboardMatrix_ = Matrix4x4::Identity();
    }
}

void Particle::Finalize()
{
    /// リソースの解放
    instancingResource_.Reset();
    SRVManager::GetInstance()->Deallocate(srvIndex_);
}

void Particle::DrawCall(ID3D12GraphicsCommandList* cl)
{
    /// モデルのテクスチャがアップロードされていない場合は描画しない
    if (!pModel_->IsEndLoading() || !particleData_.size()) return;

    ParticleSystem::CommandListData data = {};
    data.pVBV = &vertexBufferView_;
    data.srvHandle = srvGpuHandle_;
    data.textureSrvHandle = textureSRVHandleGPU_;
    data.vertexCount = static_cast<UINT>(vertexCount_);
    data.instanceCount = static_cast<UINT>(particleData_.size());
    data.rtvHandle = this->DrawableBase::GetRTVHandleCPU();

    pSystem_->DrawSingle(cl, data);
}

void Particle::reserve(size_t size, bool isInit)
{
    currentInstancingSize_ = static_cast<uint32_t>(size);

    CreateParticleForGPUResource();
    if (!isInit) SRVManager::GetInstance()->Deallocate(srvIndex_);
    CreateSRV();
    InitializeTransform();
}

void Particle::emplace_back(const ParticleData& data)
{
    auto& newData = particleData_.emplace_back(data);
    newData.seed = pRandomGenerator_->Generate(0.0f, 10000.0f);

    if (particleData_.size() > currentInstancingSize_)
    {
        currentInstancingSize_ *= 2;
        CreateParticleForGPUResource();
        SRVManager::GetInstance()->Deallocate(srvIndex_);
        CreateSRV();
    }
}

void Particle::CreateParticleForGPUResource()
{
    /// 座標変換行列リソースを作成
    instancingResource_.Reset();
    instancingResource_ = DX12Helper::CreateBufferResource(pDevice_, sizeof(ParticleForGPU) * currentInstancingSize_);
    instancingResource_->Map(0, nullptr, reinterpret_cast<void**>(&instancingData_));
    /// 座標変換行列データを初期化
    for (uint32_t index = 0; index < currentInstancingSize_; ++index)
    {
        instancingData_[index].wvp = Matrix4x4::Identity();
        instancingData_[index].world = Matrix4x4::Identity();
        instancingData_[index].color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    }
}

void Particle::CreateSRV()
{
    SRVManager* srvManager = SRVManager::GetInstance();
    srvIndex_ = srvManager->Allocate();
    srvCpuHandle_ = srvManager->GetCPUDescriptorHandle(srvIndex_);
    srvGpuHandle_ = srvManager->GetGPUDescriptorHandle(srvIndex_);

    srvManager->CreateForStructuredBuffer(srvIndex_, instancingResource_.Get(), static_cast<UINT>(currentInstancingSize_), sizeof(ParticleForGPU));
    return;
}

void Particle::GetModelData()
{
    vertexCount_ = pModel_->GetVertexCount();
    vertexBufferView_ = pModel_->GetVertexBufferView();
    textureSRVHandleGPU_ = pModel_->GetTextureSrvHandle();
}

void Particle::InitializeTransform()
{
    for (auto& datum : particleData_)
    {
        EulerTransform& transform = datum.transform;
        transform.scale = Vector3(1.0f, 1.0f, 1.0f);
        transform.rotate = Vector3(0.0f, 0.0f, 0.0f);
        transform.translate = Vector3(0.0f, 0.0f, 0.0f);
    }
}

void Particle::ParticleDataUpdate(std::list<ParticleData>::iterator itr)
{
    bool isGround = false;
    float deltaTime = DeltaTimeManager::GetInstance()->GetDeltaTime(static_cast<uint32_t>(DeltaTimeChannelReserved::Particle));

    TimeMeasurerByDt&   timer = itr->timer;
    EulerTransform&     transform = itr->transform;
    Vector3&            velocity = itr->velocity;

    float               frictionCoef = itr->frictionCoef;

    Vector4&            currentColor = itr->currentColor;
    const auto&         colorRange = itr->colorRange;

    const float         lifeTime = itr->lifeTime;
    float&              currentLifeTime = itr->currentLifeTime;
    bool&               enableCollisionFloor = itr->enableCollisionFloor;
    float               radius = itr->radius;
    v3::CollisionFloor& collisionFloor = itr->collisionFloor;
    

    /// タイマーの更新
    if (!timer.GetIsStart())
    {
        timer.Start();
        currentColor = colorRange.start;
    }

    /// 経過時間の取得
    timer.Update(static_cast<uint32_t>(DeltaTimeChannelReserved::Particle));
    currentLifeTime = lifeTime - timer.GetNow<float>();
    if (currentLifeTime < 0.0f) currentLifeTime = 0.0f;

    /// 位置の更新
    this->ParticlePositionUpdate(itr, deltaTime);

    /// 色の更新
    this->ParticleColorUpdate(itr);

    /// スケールの更新
    this->ParticleScaleUpdate(itr);

    // 当たり判定(座標計算後に実行する)
    if (enableCollisionFloor)
    {
        radius *= transform.scale.y;
        isGround = UpdateByCollisionFloor(transform.translate, velocity, collisionFloor, radius);
    }

    // 摩擦を適用
    ApplyFriction(velocity, isGround, frictionCoef, deltaTime);
}

void Particle::ParticlePositionUpdate(std::list<ParticleData>::iterator& itr, float deltaTime)
{
    EulerTransform& transform = itr->transform;
    Vector3& velocity = itr->velocity;
    Vector3& acceleration = itr->acceleration;
    Vector3& gravity = itr->accGravity;
    Vector3& resistance = itr->accResistance;
    bool& enableDirectionByVelocity = itr->enableDirectionByVelocity;
    bool& enableSmoothRandom = itr->enableSmoothRandom;
    float seed = itr->seed;
    float smoothPower = itr->smoothPower;
    float time = itr->timer.GetNow<float>();
    float speed = itr->speed_;

    if (enableSmoothRandom)
    {
        // 初期速度を direction から一度だけ設定
        if (velocity.Length() == 0.0f)
        {
            Vector3 dir = itr->direction.Normalized();
            velocity = dir * (speed > 0.0f ? speed : 1.0f);
        }

        // ノイズ生成（滑らかな変化）
        float yawNoise = Math::smoothNoise(time * 0.7f + seed * 1.13f); // -1..1
        float pitchNoise = Math::smoothNoise(time * 0.5f + seed * 2.27f); // -1..1

        // 回転角へ変換（強度は smoothPower）
        float yawDelta = yawNoise * smoothPower * deltaTime;
        float pitchDelta = pitchNoise * smoothPower * deltaTime;

        // 回転行列（Yaw -> Pitch）
        Matrix4x4 rotYaw = Matrix4x4::RotateYMatrix(yawDelta);
        Matrix4x4 rotPitch = Matrix4x4::RotateXMatrix(pitchDelta);
        Matrix4x4 rot = rotYaw * rotPitch;

        // 長さ維持して回転
        float speedLen = velocity.Length();
        if (speedLen > 0.0f)
        {
            Vector3 dirNorm = velocity.Normalized();
            dirNorm = Math::TransformNormal(dirNorm, rot);
            velocity = dirNorm * speedLen;
        }
    }

    velocity += acceleration * deltaTime;
    velocity += gravity * deltaTime;
    velocity -= resistance * deltaTime;
    if (enableDirectionByVelocity)
    {
        transform.rotate = velocity.Normalized();
    }
    transform.translate += velocity * deltaTime;
    acceleration = {};
}

void Particle::ParticleColorUpdate(std::list<ParticleData>::iterator& itr)
{
    float& alphaDeltaValue = itr->alphaDeltaValue;
    Vector4& currentColor = itr->currentColor;
    const auto& colorRange = itr->colorRange;
    const float lifeTime = itr->lifeTime;
    float& currentLifeTime = itr->currentLifeTime;

    float t = 0.0f;
    if (lifeTime != 0.0f) t = 1.0f - currentLifeTime / lifeTime;

    if (alphaDeltaValue == 0)
    {
        currentColor.Lerp(colorRange.start, colorRange.end, Math::Easing::EaseOutCubic(t));
    }
    else
    {
        Vector3 rgb = currentColor.xyz();
        rgb.Lerp(colorRange.start.xyz(), colorRange.end.xyz(), Math::Easing::EaseOutCubic(t));
        currentColor.w += alphaDeltaValue;
        currentColor = { rgb.x, rgb.y, rgb.z, currentColor.w };
    }
}

void Particle::ParticleScaleUpdate(std::list<ParticleData>::iterator& itr)
{
    const float lifeTime = itr->lifeTime;
    const float scaleDelayTime = itr->scaleDelayTime;
    float& currentLifeTime = itr->currentLifeTime;
    EulerTransform& transform = itr->transform;
    const auto& scaleRange = itr->scaleRange;

    if (lifeTime - scaleDelayTime != 0.0f)
    {
        if (currentLifeTime > lifeTime - scaleDelayTime)
        {
            transform.scale = scaleRange.start;
        }
        else
        {
            transform.scale.Lerp(scaleRange.start, scaleRange.end, 1.0f - currentLifeTime / (lifeTime - scaleDelayTime));
        }
    }
    else
    {
        transform.scale = scaleRange.start;
    }

}

void Particle::ImGui()
{
#if defined _DEBUG

    ImGui::Checkbox("Enable Billboard", &enableBillboard_);

    auto pFunc = [&]()
    {
        ImGuiTemplate::VariableTableRow("最大許容数", currentInstancingSize_);
        ImGuiTemplate::VariableTableRow("現在召喚されている数", particleData_.size());
    };

    ImGuiTemplate::VariableTable("Particle", pFunc);

#endif
}

bool Particle::UpdateByCollisionFloor(Vector3& position, Vector3& velocity, const v3::CollisionFloor& floor, float radius)
{
    if (position.y - radius < floor.elevation && velocity.y < 0.0f)
    {
        position.y = floor.elevation + radius / 2.0f;
        velocity.y = -velocity.y * floor.bounce_power;
        return true;
    }
    return false;
}

void Particle::ApplyFriction(Vector3& velocity, bool isGround, float frictionCoef, float deltaTime)
{
    if (!isGround) return;

    // XZ 平面に摩擦を適用（Y方向の速度はジャンプや重力のため残す）
    velocity.x *= std::pow(1.0f - frictionCoef, deltaTime);
    velocity.z *= std::pow(1.0f - frictionCoef, deltaTime);
}

bool Particle::DeleteParticleByCondition(std::list<ParticleData>::iterator& itr)
{
    bool isDelete = false;

    switch (itr->deleteCondition)
    {
    case ParticleDeleteCondition::LifeTime:
        isDelete = DeleteByLifeTime(itr);
        break;
    case ParticleDeleteCondition::ZeroAlpha:
        isDelete = DeleteByZeroAlpha(itr);
        break;
    default:
        break;
    }

    return isDelete;
}

bool Particle::DeleteByLifeTime(std::list<ParticleData>::iterator& itr)
{
    bool isDelete = false;

    if (itr->currentLifeTime <= 0.0f)
    {
        itr = particleData_.erase(itr);
        isDelete = true;
    }

    return isDelete;
}

bool Particle::DeleteByZeroAlpha(std::list<ParticleData>::iterator& itr)
{
    bool isDelete = false;

    if (itr->currentColor.w <= 0.0f)
    {
        itr = particleData_.erase(itr);
        isDelete = true;
    }

    return isDelete;
}

