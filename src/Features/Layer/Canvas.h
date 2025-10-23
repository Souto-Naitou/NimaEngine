#pragma once

#include <Core/DirectX12/DX12Resource/DX12Resource.h>
#include <string>
#include <vector>
#include <Features/Sprite/Sprite.h>
#include <memory>
#include <DebugTools/DebugEntry/DebugEntry.h>
#include <Core/DirectX12/PostEffectExecuter.h>
#include <d3d12.h>

struct CanvasInitParams
{
    std::string name = "Canvas";
    DirectX12* pDx12 = nullptr;
};

/// <summary>
/// キャンバスクラス
/// </summary>
class Canvas
{
public:
    Canvas() = default;
    ~Canvas() = default;

    // Not copyable
    Canvas(const Canvas&) = delete;
    Canvas& operator=(const Canvas&) = delete;
    // Not movable
    Canvas(Canvas&&) = delete;
    Canvas& operator=(Canvas&&) = delete;

    // Setters
    inline void SetEnable(bool enable) { isEnabled_ = enable; }
    void SetName(const std::string& name);

    /// <summary>
    /// 描画対象のスプライトを登録します。
    /// </summary>
    /// <param name="sprite">登録するスプライト。</param>
    /// <returns>自身への参照。</returns>
    Canvas& RegisterDrawable(Sprite* sprite);

    // Getters
    [[nodiscard]]
    inline bool IsEnabled() const { return isEnabled_; }
    [[nodiscard]]
    inline const DX12Resource& GetResource() const { return resource_; }
    [[nodiscard]]
    inline PostEffectExecuter& GetPostEffectExecuter() const { return *pPostEffectExecuter_.get(); }
    [[nodiscard]]
    inline ID3D12GraphicsCommandList* GetCommandList() const
    {
        return pPostEffectExecuter_->GetCommandList();
    }

    /// <summary>
    /// キャンバスを初期化し、内部リソースを確保します。
    /// </summary>
    /// <param name="name">キャンバス名。</param>
    void Initialize(const CanvasInitParams& params);
    
    /// <summary>
    /// 登録されたスプライトとポストエフェクトを用いて描画します。
    /// </summary>
    void Draw() const;
    
    /// <summary>
    /// デバッグ用 UI を描画します。
    /// </summary>
    void ImGui();

private:
    DX12Resource resource_;
    bool isEnabled_ = false;

    std::vector<Sprite*> sprites_;
    std::unique_ptr<DebugEntry<Canvas>> pDebugEntry_ = nullptr;
    std::unique_ptr<PostEffectExecuter> pPostEffectExecuter_ = nullptr;
};