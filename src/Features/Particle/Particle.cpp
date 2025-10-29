#include "Particle.h"
#include <Core/DirectX12/Helper/DX12Helper.h>
#include <Core/DirectX12/SRVManager.h>
#include <numbers>
#include <Common/define.h>

#if defined _DEBUG
#include <imgui.h>
#include <DebugTools/DebugManager/DebugManager.h>
#include <DebugTools/ImGuiTemplates/ImGuiTemplates.h>
#endif

using namespace Type::ParticleEmitter;

void Particle::Initialize(IModel* _pModel)
{
#if defined _DEBUG
    pDebugEntry_ = std::make_unique<DebugEntry<Particle>>("Particle", "unnamed", this, true);
#endif

    /// 必要なインスタンスを取得
    pDevice_ = pDx12_->GetDevice();
    pSystem_ = ParticleSystem::GetInstance();

    /// デフォルトのGameEyeを取得
    pGameEye_ = pSystem_->GetGlobalEye();

    if (!particleData_.capacity()) reserve(1, true);

    /// モデルを読み込む
    pModel_ = _pModel;
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

        EulerTransform& transform = itr->transform_;
        Vector4& currentColor = itr->currentColor_;

        /// パーティクルデータの更新
        ParticleDataUpdate(itr);

        /// パーティクルの条件付き削除
        if (ParticleDeleteByCondition(itr))
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
    return;
}

void Particle::Draw()
{
    /// モデルのテクスチャがアップロードされていない場合は描画しない
    if (!pModel_->IsEndLoading()) return;

    ParticleSystem::CommandListData data = {};
    data.pVBV = &vertexBufferView_;
    data.srvHandle = srvGpuHandle_;
    data.textureSrvHandle = textureSRVHandleGPU_;
    data.vertexCount = static_cast<UINT>(vertexCount_);
    data.instanceCount = static_cast<UINT>(particleData_.size());

    pSystem_->AddCommandListData(data);
}

void Particle::reserve(size_t _size, bool _isInit)
{
    auto size = sizeof(ParticleData);
    size;

    particleData_.reserve(_size);
    CreateParticleForGPUResource();
    if (!_isInit) SRVManager::GetInstance()->Deallocate(srvIndex_);
    CreateSRV();
    InitializeTransform();
    return;
}

void Particle::emplace_back(const ParticleData& _data)
{
    particleData_.emplace_back(_data);
    if (particleData_.capacity() > currentInstancingSize_)
    {
        CreateParticleForGPUResource();
        SRVManager::GetInstance()->Deallocate(srvIndex_);
        CreateSRV();
    }
}

void Particle::CreateParticleForGPUResource()
{
    /// 座標変換行列リソースを作成
    instancingResource_.Reset();
    instancingResource_ = DX12Helper::CreateBufferResource(pDevice_, sizeof(ParticleForGPU) * particleData_.capacity());
    instancingResource_->Map(0, nullptr, reinterpret_cast<void**>(&instancingData_));
    /// 座標変換行列データを初期化
    for (uint32_t index = 0; index < particleData_.capacity(); ++index)
    {
        instancingData_[index].wvp = Matrix4x4::Identity();
        instancingData_[index].world = Matrix4x4::Identity();
        instancingData_[index].color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    }
    currentInstancingSize_ = static_cast<uint32_t>(particleData_.capacity());
}

void Particle::CreateSRV()
{
    SRVManager* srvManager = SRVManager::GetInstance();
    srvIndex_ = srvManager->Allocate();
    srvCpuHandle_ = srvManager->GetCPUDescriptorHandle(srvIndex_);
    srvGpuHandle_ = srvManager->GetGPUDescriptorHandle(srvIndex_);

    srvManager->CreateForStructuredBuffer(srvIndex_, instancingResource_.Get(), static_cast<UINT>(particleData_.capacity()), sizeof(ParticleForGPU));
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
        EulerTransform& transform = datum.transform_;
        transform.scale = Vector3(1.0f, 1.0f, 1.0f);
        transform.rotate = Vector3(0.0f, 0.0f, 0.0f);
        transform.translate = Vector3(0.0f, 0.0f, 0.0f);
    }
    return;
}

void Particle::ParticleDataUpdate(std::vector<ParticleData>::iterator& _itr)
{
    bool isGround = false;
    float deltaTime = 1.0f / 60.0f;

    TimeMeasurer&       timer = _itr->timer_;

    EulerTransform&     transform = _itr->transform_;
    Vector3&            velocity = _itr->velocity_;
    Vector3&            acceleration = _itr->acceleration_;
    Vector3&            gravity = _itr->accGravity_;
    Vector3&            resistance = _itr->accResistance_;
    float               frictionCoef = _itr->frictionCoef_;

    Vector4&            currentColor = _itr->currentColor_;
    const auto&         colorRange = _itr->colorRange_;

    const auto&         scaleRange = _itr->scaleRange_;
    const float         lifeTime = _itr->lifeTime_;
    const float         scaleDelayTime = _itr->scaleDelayTime_;
    float&              currentLifeTime = _itr->currentLifeTime_;
    float&              alphaDeltaValue = _itr->alphaDeltaValue_;
    bool&               enableDirectionByVelocity = _itr->enableDirectionByVelocity;
    bool&               enableCollisionFloor = _itr->enableCollisionFloor;
    float               radius = _itr->radius;
    v3::CollisionFloor& collisionFloor = _itr->collisionFloor_;
    

    /// タイマーの更新
    if (!timer.GetIsStart())
    {
        timer.Start();
        currentColor = colorRange.start();
    }

    /// 経過時間の取得
    currentLifeTime = lifeTime - timer.GetNow<float>();
    if (currentLifeTime < 0.0f) currentLifeTime = 0.0f;

    float t = 0.0f;
    if (lifeTime != 0.0f) t = 1.0f - currentLifeTime / lifeTime;

    /// 位置の更新
    velocity += acceleration * deltaTime;
    velocity += gravity * deltaTime;
    velocity -= resistance * deltaTime;
    if (enableDirectionByVelocity)
    {
        transform.rotate = velocity.Normalize();
    }
    transform.translate += velocity * deltaTime;

    /// 加速度のリセット
    acceleration = {};

    /// 色の更新
    {
        if (alphaDeltaValue == 0)
        {
            currentColor.Lerp(colorRange.start(), colorRange.end(), EaseOutCubic(t));
        }
        else
        {
            Vector3 rgb = currentColor.xyz();
            rgb.Lerp(colorRange.start().xyz(), colorRange.end().xyz(), EaseOutCubic(t));
            currentColor.w += alphaDeltaValue;
            currentColor = { rgb.x, rgb.y, rgb.z, currentColor.w };
        }
    }


    /// スケールの更新
    /// lifetime = 10.0
    /// current = 9.0
    /// delay = 3.0
    if (lifeTime - scaleDelayTime != 0.0f)
    {
        if (currentLifeTime > lifeTime - scaleDelayTime)
        {
            transform.scale = scaleRange.start();
        }
        else
        {
            transform.scale.Lerp(scaleRange.start(), scaleRange.end(), 1.0f - currentLifeTime / (lifeTime - scaleDelayTime));
        }
    }
    else
    {
        transform.scale = scaleRange.start();
    }

    // 当たり判定(座標計算後に実行する)
    if (enableCollisionFloor)
    {
        radius *= transform.scale.y;
        isGround = UpdateByCollisionFloor(transform.translate, velocity, collisionFloor, radius);
    }

    // 摩擦を適用
    ApplyFriction(velocity, isGround, frictionCoef, deltaTime);

    return;
}

void Particle::ImGui()
{
#if defined _DEBUG

    ImGui::Checkbox("Enable Billboard", &enableBillboard_);

    auto pFunc = [&]()
    {
        ImGuiTemplate::VariableTableRow("最大許容数", particleData_.capacity());
        ImGuiTemplate::VariableTableRow("現在召喚されている数", particleData_.size());
    };

    ImGuiTemplate::VariableTable("Particle", pFunc);

#endif
}

float Particle::EaseOutCubic(float t)
{
    return 1.0f - std::powf(1.0f - t, 3.0f);
}

float Particle::EaseOutQuad(float t)
{
    return 1.0f - std::powf(1.0f - t, 2.0f);
}

bool Particle::UpdateByCollisionFloor(Vector3& _position, Vector3& _velocity, const v3::CollisionFloor& _floor, float _radius)
{
    if (_position.y - _radius < _floor.elevation && _velocity.y < 0.0f)
    {
        _position.y = _floor.elevation + _radius / 2.0f;
        _velocity.y = -_velocity.y * _floor.bounce_power;
        return true;
    }
    return false;
}

void Particle::ApplyFriction(Vector3& _velocity, bool _isGround, float _frictionCoef, float _deltaTime)
{
    if (!_isGround) return;

    // XZ 平面に摩擦を適用（Y方向の速度はジャンプや重力のため残す）
    _velocity.x *= std::pow(1.0f - _frictionCoef, _deltaTime);
    _velocity.z *= std::pow(1.0f - _frictionCoef, _deltaTime);
}

bool Particle::ParticleDeleteByCondition(std::vector<ParticleData>::iterator& _itr)
{
    bool isDelete = false;

    switch (_itr->deleteCondition_)
    {
    case ParticleDeleteCondition::LifeTime:
        isDelete = DeleteByLifeTime(_itr);
        break;
    case ParticleDeleteCondition::ZeroAlpha:
        isDelete = DeleteByZeroAlpha(_itr);
        break;
    default:
        break;
    }

    return isDelete;
}

bool Particle::DeleteByLifeTime(std::vector<ParticleData>::iterator& _itr)
{
    bool isDelete = false;

    if (_itr->currentLifeTime_ <= 0.0f)
    {
        _itr = particleData_.erase(_itr);
        isDelete = true;
    }

    return isDelete;
}

bool Particle::DeleteByZeroAlpha(std::vector<ParticleData>::iterator& _itr)
{
    bool isDelete = false;

    if (_itr->currentColor_.w <= 0.0f)
    {
        _itr = particleData_.erase(_itr);
        isDelete = true;
    }

    return isDelete;
}

