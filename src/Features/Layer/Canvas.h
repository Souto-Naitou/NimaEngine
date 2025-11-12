#pragma once

#include <Core/DirectX12/DX12Resource/DX12Resource.h>
#include <string>
#include <vector>
#include <memory>
#include <DebugTools/DebugEntry/DebugEntry.h>
#include <Core/DirectX12/PostEffectExecuter.h>
#include <d3d12.h>
#include <Features/Cubemap/Skybox.h>
#include <drawable/base/DrawableBase.h>
#include <DebugTools/ImGuiManager/ImGuiManager.h>



/// <summary>
/// キャンバスクラス
/// </summary>
class Canvas
{
public:
    struct Params
    {
        std::string name = "Canvas";
        DirectX12* pDx12 = nullptr;
        CubemapSystem* pCubemapSystem = nullptr;

        #ifdef _DEBUG
        ImGuiManager* pImGuiManager = nullptr;
        #endif // _DEBUG
    };

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
    /// 描画対象のスプライトを登録します
    /// </summary>
    /// <param name="sprite">登録するスプライト</param>
    /// <returns>自身への参照</returns>
    Canvas& RegisterDrawable(DrawableBase* sprite);

    /// <summary>
    /// 描画対象のスカイボックスを登録します。
    /// </summary>
    /// <param name="skybox">登録するスカイボックス</param>
    /// <returns>自身への参照</returns>
    Canvas& RegisterDrawable(Skybox* skybox);

    void UnregisterDrawable(Skybox* skybox);
    void UnregisterDrawable(DrawableBase* drawable);

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
    void Initialize(const Canvas::Params& params);

    void Finalize();

    /// <summary>
    /// 登録されたオブジェクトを描画します。
    /// </summary>
    void DrawObjects();

    /// <summary>
    /// ポストエフェクトを適用します。
    /// </summary>
    void ApplyPostEffects();

    /// <summary>
    /// Canvasを描画します。
    /// </summary>
    void Draw() const;
    
    /// <summary>
    /// デバッグ用 UI を描画します。
    /// </summary>
    void ImGui();

private:
    void ParameterCheck(const Canvas::Params& params) const;

    DX12Resource resource_ = {};
    bool isEnabled_ = true;

    Canvas::Params params_ = {};
    std::list<DrawableBase*> drawables_;
    std::list<Skybox*> skyboxes_;


    std::unique_ptr<DebugEntry<Canvas>> pDebugEntry_ = nullptr;
    std::unique_ptr<PostEffectExecuter> pPostEffectExecuter_ = nullptr;
};