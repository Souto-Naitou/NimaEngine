#pragma once

#include <Effects/SceneTransition/TransBase.h>
#include <string>
#include <Features/Sprite/Sprite.h>

/// <summary>
/// フェードイン・フェードアウト
/// </summary>
class TransFadeInOut : public TransBase
{
public:
    ~TransFadeInOut();
    /// <summary>
    /// フェードイン・アウトの初期化を行います。
    /// </summary>
    /// <param name="_sceneName">遷移先のシーン名。</param>
    void Initialize(const std::string& _sceneName) override;
    /// <summary>
    /// 状態を更新します。
    /// </summary>
    void Update() override;
    /// <summary>
    /// エフェクトを描画します。
    /// </summary>
    void Draw() override;
    /// <summary>
    /// 後始末を行います。
    /// </summary>
    void Finalize() override;
    /// <summary>
    /// デバッグUIを描画します。
    /// </summary>
    void ImGui() override;

private:

    std::string name_;

    float opacity_ = 0.0f;
    bool isChangedScene_ = false;

    double duration_ = 1.0;

    uint32_t countPhase_ = 0;

    std::string sceneName_;
    std::unique_ptr<Sprite> sprite_;
};