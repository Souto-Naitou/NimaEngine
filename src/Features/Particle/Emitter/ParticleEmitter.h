#pragma once

#pragma warning(disable: 4324) // 構造体のメンバがアライメントのためにパディングされる

#include <Features/TimeMeasurer/TimeMeasurer.h>
#include <Features/Particle/Particle.h>
#include <Features/Primitive/AABB.h>
#include <WinTools/WinTools.h>
#include <Features/GameEye/GameEye.h>
#include <Vector3.h>
#include <Vector4.h>
#include <string>
#include <memory>
#include <cstdint>
#include <Features/Particle/Emitter/EmitterData.h>

/// <summary>
/// パーティクルエミッタ
/// </summary>
class ParticleEmitter
{
public:
    ParticleEmitter() = default;
    ~ParticleEmitter() = default;

    void Initialize(IModel* _pModel, const std::string& _jsonPath = {});
    void EnableManualMode();
    void DisableManualMode();
    void Update();
    void Draw();
    void Finalize();

    void Emit();

public: /// Setter
    void SetPosition(const Vector3& _position) { position_ = _position; }
    void SetGameEye(GameEye** _eye) { this->ModifyGameEye(_eye); }
    void SetEnableBillboard(bool _enable) { particle_->SetEnableBillboard(_enable); }

public: /// Getter
    //EmitterData& GetEmitterData() { return emitterData_; }

private:
    using EmitterData = Type::ParticleEmitter::v3::Data;

    static constexpr uint32_t   kDefaultReserveCount_   = 6000u;
    std::string                 name_                   = {};               // 名前
    std::string                 ptrHex_                 = {};               // ポインタの16進数表記
    std::string                 particleName_           = {};               // 名前
    std::string                 jsonPath_               = {};               // JSONファイルパス
    TimeMeasurer                timer_                  = {};               // 計測用タイマー
    TimeMeasurer                reloadTimer_            = {};               // リロード用タイマー
    double                      reloadInterval_         = 1.0;              // リロード間隔
    EmitterData                 emitterData_            = {};               // エミッタデータ
    EmitterData                 fromJsonData_           = {};
    Particle*                   particle_               = nullptr;
    std::unique_ptr<AABB>       aabb_                   = nullptr;
    bool                        jsonFileExist_          = true;
    WinTools*                   winTools_               = nullptr;
    bool                        isManualMode_           = false;
    bool                        isEmitRequest_          = false;
    Vector3                     position_               = {};


private:
    void EmitParticle();
    Vector3 RandomUnitSphere();

private:
    void ImGui();
    void ModifyGameEye(GameEye** _eye);
};