#include "ParticleEmitter.h"

#ifdef _DEBUG
#include <DebugTools/DebugManager/DebugManager.h>
#include <imgui.h>
#include <Utility/String/strutl.h>
#endif // _DEBUG

#include <drawable/particle/ParticleStorage.h>
#include <Features/RandomGenerator/RandomGenerator.h>
#include <drawable/particle/Manager/EmitterManager.h>
#include <Core/ConfigManager/ConfigManager.h>
#include "EmitterData.h"
#include <WinTools/WinTools.h>
#include <Range.h>
#include <numbers>
#include <algorithm>

const uint32_t ParticleEmitter::kDefaultReserveCount_;

void ParticleEmitter::Initialize(const ParticleEmitter::Params& params)
{
    debugEntry_ = std::make_unique<DebugEntry<ParticleEmitter>>("ParticleEmitter", "ParticleEmitter", this, false);

    winTools_ = WinTools::GetInstance();

    if (params.jsonPath.empty())
    {
        jsonPath_.clear();
    }
    else
    {
        const std::filesystem::path pathParam = params.jsonPath;
        const std::filesystem::path cfgPath = ConfigManager::GetInstance()->GetConfigData().particle_emitter_paths.back();
        jsonPath_ = cfgPath / pathParam;
    }

    // タイマースタート
    timer_.Start();
    reloadTimer_.Start();

    // パーティクル初期化
    particle_ = params.particle;
    
    bool isExist = std::filesystem::directory_entry(jsonPath_).exists();

    if (!isExist)
    {
        fromJsonData_ = {};
        emitterData_ = {};
    }
    else
    {
        fromJsonData_ = EmitterManager::GetInstance()->LoadFile(jsonPath_.string());
        emitterData_ = fromJsonData_;
    }


    if (!emitterData_.name.empty())
    {
        particleName_ = emitterData_.name;
        debugEntry_->SetName(particleName_);
        particle_->SetName(particleName_);
    }

    aabb_ = std::make_unique<AABB>();
    aabb_->Initialize();
}

void ParticleEmitter::EnableManualMode()
{
    isManualMode_ = true;
}

void ParticleEmitter::DisableManualMode()
{
    isManualMode_ = false;
}

void ParticleEmitter::Update()
{
    if (timer_.GetNow<float>() > emitterData_.common.emitInterval)
    {
        if (emitterData_.common.emitNum < 0)
        {
            emitterData_.common.emitNum = 0;
        }
        for (int32_t i = 0; i < emitterData_.common.emitNum; ++i)
        {
            if (!isManualMode_)
            {
                EmitParticle();
            }
        }
        isEmitRequest_ = false;
        timer_.Reset();
        timer_.Start();
    }

    emitterData_ = fromJsonData_;
    emitterData_.common.emitPositionFixed = fromJsonData_.common.emitPositionFixed + position_;
    emitterData_.ranges.position.start = fromJsonData_.ranges.position.start + position_;
    emitterData_.ranges.position.end = fromJsonData_.ranges.position.end + position_;

    aabb_->SetMinMax(emitterData_.ranges.position.start, emitterData_.ranges.position.end);
}

void ParticleEmitter::Draw1F()
{
#ifdef _DEBUG
    if (isDrawLine_ && emitterData_.flags.enableRandomEmit)
    {
        aabb_->Draw1F();
    }
#endif // _DEBUG
}

void ParticleEmitter::EmitParticle()
{
    particle_->emplace_back({});
    auto& parameter = particle_->GetParticleData();
    auto& datum = parameter.back();

    /// 初期トランスフォーム
    this->InitTransform(datum);

    /// 衝突半径
    datum.radius = emitterData_.common.radius;

    /// スケール
    this->InitScale(datum);
    
    /// ライフタイム
    datum.lifeTime = emitterData_.common.particleLifeTime;

    /// 速度
    this->InitVelocity(datum);

    /// 回転
    this->InitRotation(datum);

    // 色範囲
    datum.colorRange = emitterData_.ranges.color;

    // アルファ値の変化量
    datum.alphaDeltaValue = emitterData_.common.alphaDeltaValue;
    // 消去条件
    datum.deleteCondition = ParticleDeleteCondition::LifeTime;
    // 物理
    datum.accGravity = emitterData_.physics.gravity;
    datum.accResistance = emitterData_.physics.resistance;
    datum.frictionCoef = emitterData_.physics.frictionCoef;
    datum.smoothPower = emitterData_.physics.smoothNoisePower;

    // 衝突床
    datum.enableCollisionFloor = emitterData_.flags.enableCollisionFloor;
    datum.collisionFloor = emitterData_.collisionFloor;
    datum.enableSmoothRandom = emitterData_.flags.enableSmoothNoise;

    aabb_->SetMinMax(emitterData_.ranges.position.start, emitterData_.ranges.position.end);
}

void ParticleEmitter::InitTransform(ParticleData& datum)
{
    if (emitterData_.flags.enableRandomEmit)
    {
        datum.transform.translate = Vector3(
            pRandGen_->Generate(emitterData_.ranges.position.start.x, emitterData_.ranges.position.end.x),
            pRandGen_->Generate(emitterData_.ranges.position.start.y, emitterData_.ranges.position.end.y),
            pRandGen_->Generate(emitterData_.ranges.position.start.z, emitterData_.ranges.position.end.z));
    }
    else
    {
        datum.transform.translate = emitterData_.common.emitPositionFixed;
    }
}

void ParticleEmitter::InitScale(ParticleData& datum)
{
    if (emitterData_.flags.enableRandomScale)
    {
        datum.transform.scale = Vector3(
            pRandGen_->Generate(emitterData_.ranges.scaleRandom.start.x, emitterData_.ranges.scaleRandom.end.x),
            pRandGen_->Generate(emitterData_.ranges.scaleRandom.start.y, emitterData_.ranges.scaleRandom.end.y),
            pRandGen_->Generate(emitterData_.ranges.scaleRandom.start.z, emitterData_.ranges.scaleRandom.end.z)
        );
        datum.scaleRange.start = datum.transform.scale;
        if (!emitterData_.flags.enableScaleTransition)
        {
            datum.scaleRange.end = datum.transform.scale;
        }
    }
    else if (emitterData_.flags.enableScaleTransition)
    {
        datum.transform.scale = emitterData_.ranges.scale.start;
        datum.scaleRange = emitterData_.ranges.scale;
    }
    else
    {
        datum.transform.scale = emitterData_.common.scaleFixed;
        datum.scaleRange = Range(emitterData_.common.scaleFixed, emitterData_.common.scaleFixed);
    }
    datum.scaleDelayTime = emitterData_.common.scaleDelayTime;
}

void ParticleEmitter::InitVelocity(ParticleData& datum)
{
    if (emitterData_.flags.enableRandomVelocity)
    {
        if (emitterData_.flags.velocityDistribution == VelocityDistribution::Box)
        {
            datum.velocity = Vector3(
                pRandGen_->Generate(emitterData_.ranges.velocityRandom.start.x, emitterData_.ranges.velocityRandom.end.x),
                pRandGen_->Generate(emitterData_.ranges.velocityRandom.start.y, emitterData_.ranges.velocityRandom.end.y),
                pRandGen_->Generate(emitterData_.ranges.velocityRandom.start.z, emitterData_.ranges.velocityRandom.end.z)
            );
        }
        else if (emitterData_.flags.velocityDistribution == VelocityDistribution::Sphere)
        {
            // 球面上のランダムなベクトルを生成
            Vector3 randomDirection = RandomUnitSphere();
            float speed = pRandGen_->Generate(emitterData_.ranges.velocityRandomSphere.start, emitterData_.ranges.velocityRandomSphere.end);
            datum.velocity = randomDirection * speed;
        }
    }
    else
    {
        datum.velocity = emitterData_.common.velocityFixed;
    }

    datum.direction = datum.velocity;
    datum.speed_ = datum.velocity.Length();
}

void ParticleEmitter::InitRotation(ParticleData& datum)
{
    if (emitterData_.flags.enableRandomRotation)
    {
        datum.transform.rotate = Vector3(
            pRandGen_->Generate(emitterData_.ranges.rotationRandom.start.x, emitterData_.ranges.rotationRandom.end.x),
            pRandGen_->Generate(emitterData_.ranges.rotationRandom.start.y, emitterData_.ranges.rotationRandom.end.y),
            pRandGen_->Generate(emitterData_.ranges.rotationRandom.start.z, emitterData_.ranges.rotationRandom.end.z)
        );
    }
    else
    {
        datum.transform.rotate = {};
    }
}

void ParticleEmitter::ImGuiSectionCommon()
{
    #ifdef _DEBUG
    char path[512] = "";
    char name[128] = "";
    std::string pathString = jsonPath_.string();
    strncpy_s(path, sizeof(path), pathString.c_str(), _TRUNCATE);
    std::string nameString = particleName_;
    strncpy_s(name, sizeof(name), particleName_.c_str(), _TRUNCATE);

    if (ImGui::CollapsingHeader("一般"))
    {
        if (ImGui::InputText("エミッター名", name, sizeof(name), ImGuiInputTextFlags_EnterReturnsTrue))
        {
            particleName_ = name;
            fromJsonData_.name = name;
            debugEntry_->SetName(particleName_);
            particle_->SetName(particleName_);
        }
        if (ImGui::Button("マニュアル発生"))
        {
            Emit();
        }
        if (ImGui::InputText("ファイルパス", path, sizeof(path)))
        {
            jsonFileExist_ = std::filesystem::exists(path);
        }

        if (ImGui::Button("保存"))
        {
            EmitterManager::GetInstance()->SaveFile(path, fromJsonData_);
        }
        ImGui::SameLine();
        if (ImGui::Button("読み込み"))
        {
            if (std::filesystem::directory_entry(path).exists())
            {
                fromJsonData_ = EmitterManager::GetInstance()->ReloadFile(path);
                /// 名前が空でないなら
                if (!fromJsonData_.name.empty())
                {
                    particleName_ = fromJsonData_.name;
                    debugEntry_->SetName(particleName_);
                    particle_->SetName(particleName_);
                }
                jsonFileExist_ = true;
            }
            else
            {
                jsonFileExist_ = false;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("参照"))
        {
            std::string temp = winTools_->OpenFileDialog();
            if (!temp.empty() && std::filesystem::path(temp).extension() == ".json")
            {
                strncpy_s(path, sizeof(path), temp.c_str(), _TRUNCATE);
                jsonFileExist_ = true;
            }
            else jsonFileExist_ = false;
        }
        if (!jsonFileExist_)
        {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "ファイルが存在しないか、拡張子がjsonではありません。");
        }

        ImGui::DragFloat("パーティクル寿命", &fromJsonData_.common.particleLifeTime, 0.1f, 0.0f, FLT_MAX);

        ImGui::DragFloat("発生間隔", &fromJsonData_.common.emitInterval, 0.02f, 0.02f, FLT_MAX);

        ImGui::InputInt("発生数", (int*)&fromJsonData_.common.emitNum);

        ImGui::Spacing();
    }

    jsonPath_ = path;

    #endif // _DEBUG
}

void ParticleEmitter::ImGuiSectionColor()
{
#ifdef _DEBUG
    if (ImGui::CollapsingHeader("色の変化"))
    {
        ImGui::ColorEdit4("開始色", &fromJsonData_.ranges.color.start.x);
        ImGui::SameLine();
        if (ImGui::Button("入れ替え##Color"))
        {
            Vector4 temp = fromJsonData_.ranges.color.start;
            fromJsonData_.ranges.color.start = fromJsonData_.ranges.color.end;
            fromJsonData_.ranges.color.end = temp;
        }

        ImGui::ColorEdit4("終了色", &fromJsonData_.ranges.color.end.x);
        ImGui::SameLine();
        if (ImGui::Button("同期##Color"))
        {
            fromJsonData_.ranges.color.end = fromJsonData_.ranges.color.start;
        }

        ImGui::SliderFloat("透明度の変化量", &fromJsonData_.common.alphaDeltaValue, -0.2f, 0.0f);


        ImGui::Spacing();
    }
#endif // _DEBUG
}

void ParticleEmitter::ImGuiSectionTransform()
{
#ifdef _DEBUG

    if (ImGui::CollapsingHeader("変形"))
    {
        ImGui::DragFloat("衝突半径", &fromJsonData_.common.radius, 0.01f, FLT_MIN, FLT_MAX);
        if (!fromJsonData_.flags.enableRandomScale && !fromJsonData_.flags.enableScaleTransition)
        {
            ImGui::DragFloat3("スケール", &fromJsonData_.common.scaleFixed.x, 0.01f);
        }

        ImGui::Checkbox("スケールのランダマイズ", &fromJsonData_.flags.enableRandomScale);
        if (fromJsonData_.flags.enableRandomScale)
        {
            ImGui::DragFloat3("スケールランダム範囲-開始", &fromJsonData_.ranges.scaleRandom.start.x, 0.01f);
            ImGui::DragFloat3("スケールランダム範囲-終了", &fromJsonData_.ranges.scaleRandom.end.x, 0.01f);
        }

        ImGui::Checkbox("スケール遷移", &fromJsonData_.flags.enableScaleTransition);
        if (fromJsonData_.flags.enableScaleTransition)
        {
            if (fromJsonData_.flags.enableRandomScale) ImGui::BeginDisabled();
            ImGui::DragFloat3("開始スケール", &fromJsonData_.ranges.scale.start.x, 0.01f);
            if (fromJsonData_.flags.enableRandomScale) ImGui::EndDisabled();

            ImGui::SameLine();
            if (ImGui::Button("入れ替え##Scale"))
            {
                Vector3 temp = fromJsonData_.ranges.scale.start;
                fromJsonData_.ranges.scale.start = fromJsonData_.ranges.scale.end;
                fromJsonData_.ranges.scale.end = temp;
            }

            ImGui::DragFloat3("終了スケール", &fromJsonData_.ranges.scale.end.x, 0.01f);
            ImGui::SameLine();
            if (ImGui::Button("同期##Scale"))
            {
                fromJsonData_.ranges.scale.end = fromJsonData_.ranges.scale.start;
            }
            ImGui::DragFloat("スケール遅延時間", &fromJsonData_.common.scaleDelayTime, 0.01f);
        }

        ImGui::Checkbox("姿勢のランダマイズ", &fromJsonData_.flags.enableRandomRotation);
        if (fromJsonData_.flags.enableRandomRotation)
        {
            ImGui::DragFloat3("姿勢ランダム範囲-開始", &fromJsonData_.ranges.rotationRandom.start.x, 0.01f);
            ImGui::DragFloat3("姿勢ランダム範囲-終了", &fromJsonData_.ranges.rotationRandom.end.x, 0.01f);
        }

        ImGui::Spacing();
    }

#endif // _DEBUG
}

void ParticleEmitter::ImGuiSectionSpawnPoint()
{
#ifdef _DEBUG

    if (ImGui::CollapsingHeader("生成場所"))
    {
        ImGui::Checkbox("ランダム範囲生成", &fromJsonData_.flags.enableRandomEmit);
        if (fromJsonData_.flags.enableRandomEmit)
        {
            ImGui::DragFloat3("発生開始地点", &fromJsonData_.ranges.position.start.x, 0.01f);
            ImGui::DragFloat3("発生終了地点", &fromJsonData_.ranges.position.end.x, 0.01f);
        }
        else
        {
            ImGui::DragFloat3("発生位置", &fromJsonData_.common.emitPositionFixed.x, 0.01f);
        }

        ImGui::Spacing();
    }

#endif // _DEBUG
}

void ParticleEmitter::ImGuiSectionVelocity()
{
#ifdef _DEBUG
    static const char* velocityDistributionTypes[] = { "Box", "Sphere" };
    int selectedVelocityDistribution = static_cast<int>(fromJsonData_.flags.velocityDistribution);

    if (ImGui::CollapsingHeader("速度"))
    {
        ImGui::Checkbox("速度のランダマイズ", &fromJsonData_.flags.enableRandomVelocity);
        const char* preview = velocityDistributionTypes[selectedVelocityDistribution];
        if (ImGui::BeginCombo("速度の方向分布タイプ", preview))
        {
            for (int n = 0; n < IM_ARRAYSIZE(velocityDistributionTypes); n++)
            {
                const bool is_selected = (selectedVelocityDistribution == n);
                if (ImGui::Selectable(velocityDistributionTypes[n], is_selected))
                    fromJsonData_.flags.velocityDistribution = static_cast<Type::ParticleEmitter::VelocityDistribution>(n);

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        bool isSphere = (fromJsonData_.flags.velocityDistribution == Type::ParticleEmitter::VelocityDistribution::Sphere);
        if (fromJsonData_.flags.enableRandomVelocity)
        {
            if (isSphere)
            {
                ImGui::DragFloat("速度ランダム範囲-球面開始", &fromJsonData_.ranges.velocityRandomSphere.start, 0.01f);
                ImGui::DragFloat("速度ランダム範囲-球面終了", &fromJsonData_.ranges.velocityRandomSphere.end, 0.01f);
            }
            else
            {
                ImGui::DragFloat3("速度ランダム範囲-開始", &fromJsonData_.ranges.velocityRandom.start.x, 0.01f);
                ImGui::DragFloat3("速度ランダム範囲-終了", &fromJsonData_.ranges.velocityRandom.end.x, 0.01f);
            }
        }
        else
        {
            ImGui::DragFloat3("速度", &fromJsonData_.common.velocityFixed.x, 0.01f);
        }

        ImGui::Spacing();
    }
#endif // _DEBUG
}

void ParticleEmitter::ImGuiSectionPhysics()
{
#ifdef _DEBUG

    if (ImGui::CollapsingHeader("物理"))
    {
        ImGui::DragFloat3("重力", &fromJsonData_.physics.gravity.x, 0.01f);
        ImGui::DragFloat3("抵抗", &fromJsonData_.physics.resistance.x, 0.01f);
        ImGui::DragFloat("動摩擦係数", &fromJsonData_.physics.frictionCoef, 0.01f);

        ImGui::Checkbox("スムースノイズ", &fromJsonData_.flags.enableSmoothNoise);
        if (fromJsonData_.flags.enableSmoothNoise)
        {
            ImGui::DragFloat("スムースノイズ強度", &fromJsonData_.physics.smoothNoisePower, 0.01f, 0.0f, FLT_MAX);
        }
    }

#endif // _DEBUG
}

void ParticleEmitter::ImGuiSectionCollisionFloor()
{
#ifdef _DEBUG
    if (ImGui::CollapsingHeader("衝突床"))
    {
        ImGui::Checkbox("衝突床との判定", &fromJsonData_.flags.enableCollisionFloor);
        if (fromJsonData_.flags.enableCollisionFloor)
        {
            ImGui::DragFloat("高さ", &fromJsonData_.collisionFloor.elevation, 0.01f);
            ImGui::DragFloat("反発係数", &fromJsonData_.collisionFloor.bounce_power, 0.01f);
        }

        ImGui::Spacing();
    }
#endif // _DEBUG
}

void ParticleEmitter::ImGuiSectionDebug()
{
    #ifdef _DEBUG
    if (ImGui::CollapsingHeader("デバッグ"))
    {
        ImGui::Checkbox("エミッタ範囲の描画", &isDrawLine_);
        ImGui::Spacing();
    }
    #endif // _DEBUG
}

Vector3 ParticleEmitter::RandomUnitSphere()
{
    float u = pRandGen_->Generate(0.0f, 1.0f);
    float v = pRandGen_->Generate(0.0f, 1.0f);
    float theta = 2.0f * std::numbers::pi_v<float> * u;
    float phi = std::acos(2.0f * v - 1.0f);

    float x = std::sin(phi) * std::cos(theta);
    float y = std::sin(phi) * std::sin(theta);
    float z = std::cos(phi);
    return Vector3(x, y, z);
}

void ParticleEmitter::ImGui()
{
#ifdef _DEBUG

    ImGui::Text("Name : %s", particleName_.c_str());

    this->ImGuiSectionCommon();

    this->ImGuiSectionColor();
    
    this->ImGuiSectionTransform();

    this->ImGuiSectionSpawnPoint();

    this->ImGuiSectionVelocity();

    this->ImGuiSectionCollisionFloor();

    this->ImGuiSectionPhysics();

    this->ImGuiSectionDebug();

#endif // _DEBUG
}

void ParticleEmitter::ModifyGameEye(IGameEye** eye)
{
    particle_->SetGameEye(eye);
    aabb_->SetGameEye(eye);
}

void ParticleEmitter::Finalize()
{
    particle_ = nullptr;
}

void ParticleEmitter::Emit()
{
    if (isManualMode_)
    {
        this->Update();
        for (int32_t i = 0; i < emitterData_.common.emitNum; ++i)
        {
            EmitParticle();
        }
    }
}