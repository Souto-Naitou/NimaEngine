#pragma once

#pragma warning(disable: 4324) // 構造体のメンバがアライメントのためにパディングされる

#include <Features/TimeMeasurer/TimeMeasurer.h>
#include <drawable/particle/Particle.h>
#include <Features/Primitive/AABB.h>
#include <WinTools/WinTools.h>
#include <Features/GameEye/GameEye.h>
#include <Vector3.h>
#include <string>
#include <memory>
#include <cstdint>
#include <drawable/particle/Emitter/EmitterData.h>
#include <DebugTools/DebugEntry/DebugEntry.h>
#include <Features/RandomGenerator/RandomGenerator.h>

struct ParticleEmitterInitParams
{
    Particle* particle = nullptr;
    std::string jsonPath = {};
};

/// <summary>
/// パーティクルエミッタ
/// </summary>
class ParticleEmitter
{
public:
    ParticleEmitter() = default;
    ~ParticleEmitter() = default;

    /// <summary>
    /// パーティクルエミッタを初期化します。
    /// </summary>
    /// <param name="params">初期化パラメータ。</param>
    void Initialize(const ParticleEmitterInitParams& params);

    /// <summary>
    /// 手動発生モードを有効にします。
    /// </summary>
    void EnableManualMode();

    /// <summary>
    /// 手動発生モードを無効にします。
    /// </summary>
    void DisableManualMode();

    /// <summary>
    /// エミッタの状態を更新します。
    /// </summary>
    void Update();

    /// <summary>
    /// パーティクルの描画を行います。
    /// </summary>
    void Draw();

    /// <summary>
    /// リソースの解放を行います。
    /// </summary>
    void Finalize();

    /// <summary>
    /// 1回分のパーティクル発生を行います。
    /// </summary>
    void Emit();

    /// <summary>
    /// デバッグUIを描画します。
    /// </summary>
    void ImGui();

public: /// Setter
    void SetPosition(const Vector3& _position) { position_ = _position; }
    void SetGameEye(GameEye** _eye) { this->ModifyGameEye(_eye); }
    void SetEnableBillboard(bool _enable) { particle_->SetEnableBillboard(_enable); }

public: /// Getter
    Particle* GetParticle() const { return particle_; }

private:
    using EmitterData = Type::ParticleEmitter::v3::Data;

    std::unique_ptr<DebugEntry<ParticleEmitter>> debugEntry_ = nullptr;
    static constexpr uint32_t   kDefaultReserveCount_   = 6000u;
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
    bool                        isManualMode_           = false;
    bool                        isEmitRequest_          = false;
    Vector3                     position_               = {};

    /// [ Pointers ]
    WinTools*                   winTools_               = nullptr;
    RandomGenerator*            pRandGen_               = RandomGenerator::GetInstance();


private:
    /// <summary>
    /// 実際のパーティクル生成処理を行います。
    /// </summary>
    void EmitParticle();

    /// [ 初期化関数群 ]
    void InitTransform(ParticleData& datum);
    void InitScale(ParticleData& datum);
    void InitVelocity(ParticleData& datum);
    void InitRotation(ParticleData& datum);

    void ImGuiSectionCommon();
    void ImGuiSectionColor();
    void ImGuiSectionTransform();
    void ImGuiSectionSpawnPoint();
    void ImGuiSectionVelocity();

    /// <summary>
    /// 単位球上のランダムベクトルを生成します。
    /// </summary>
    /// <returns>ランダム方向ベクトル。</returns>
    Vector3 RandomUnitSphere();

private:
    /// <summary>
    /// カメラ参照を更新します。
    /// </summary>
    /// <param name="_eye">GameEye のダブルポインタ。</param>
    void ModifyGameEye(GameEye** _eye);
};