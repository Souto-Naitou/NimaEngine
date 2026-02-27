#pragma once

#pragma warning(disable: 4324) // 構造体のメンバがアライメントのためにパディングされる

#include <Features/TimeMeasurer/TimeMeasurer.h>
#include <drawable/particle/Particle.h>
#include <WinTools/WinTools.h>
#include <Features/GameEye/GameEye.h>
#include <drawable/particle/Emitter/EmitterData.h>
#include <Features/RandomGenerator/RandomGenerator.h>
#include <Features/Primitive/AABB.h>
#include <DebugTools/DebugEntry/DebugEntry.h>
#include <Vector3.h>
#include <string>
#include <cstdint>
#include <memory>

/// <summary>
/// パーティクルエミッタ
/// </summary>
class ParticleEmitter
{
public:
    struct Params
    {
        Particle* particle = nullptr;
        std::string jsonPath = {};
    };

    /// <summary>
    /// パーティクルエミッタを初期化します。
    /// </summary>
    /// <param name="params">初期化パラメータ。</param>
    void Initialize(const Params& params);

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
    /// エミッタの描画を行います。
    /// </summary>
    void Draw1F();

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
    void SetPosition(const Vector3& position) { position_ = position; }
    void SetGameEye(GameEye** eye) { this->ModifyGameEye(eye); }
    void SetEnableBillboard(bool enable) { particle_->SetEnableBillboard(enable); }

public: /// Getter
    Particle* GetParticle() const { return particle_; }

private:
    using EmitterData = Type::ParticleEmitter::v3::Data;

    std::unique_ptr<DebugEntry<ParticleEmitter>> debugEntry_ = nullptr;
    static constexpr uint32_t   kDefaultReserveCount_   = 6000u;
    std::string                 particleName_           = {};               // 名前
    std::filesystem::path       jsonPath_               = {};               // JSONファイルパス
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
    bool                        isDrawLine_             = false;

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
    void ImGuiSectionPhysics();
    void ImGuiSectionCollisionFloor();
    void ImGuiSectionDebug();

    /// <summary>
    /// 単位球上のランダムベクトルを生成します。
    /// </summary>
    /// <returns>ランダム方向ベクトル。</returns>
    Vector3 RandomUnitSphere();

private:
    /// <summary>
    /// カメラ参照を更新します。
    /// </summary>
    /// <param name="eye">GameEye のダブルポインタ。</param>
    void ModifyGameEye(GameEye** eye);
};