#pragma once

#include <Core/DirectX12/DX12Resource/DX12Resource.h>
#include <string>
#include <vector>
#include <Features/Sprite/Sprite.h>
#include <memory>
#include <DebugTools/DebugEntry/DebugEntry.h>
#include <Core/DirectX12/PostEffectExecuter.h>

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

    /// 描画オブジェクトの登録
    Canvas& RegisterDrawable(Sprite* sprite);

    // Getters
    [[nodiscard]]
    inline bool IsEnabled() const { return isEnabled_; }
    [[nodiscard]]
    inline const std::string& GetName() const { return name_; }
    [[nodiscard]]
    inline const DX12Resource& GetResource() const { return resource_; }
    [[nodiscard]]
    inline PostEffectExecuter& GetPostEffectExecuter() const { return *pPostEffectExecuter_.get(); }

    void Initialize(const std::string& name);
    void Draw() const;
    void ImGui();

private:
    DX12Resource resource_;
    std::string name_ = "Canvas";
    bool isEnabled_ = false;

    std::vector<Sprite*> sprites_;
    std::unique_ptr<DebugEntry<Canvas>> pDebugEntry_ = nullptr;
    std::unique_ptr<PostEffectExecuter> pPostEffectExecuter_ = nullptr;
};