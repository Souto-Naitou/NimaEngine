#pragma once

#include <Core/DirectX12/DX12Resource/DX12Resource.h>
#include <string>
#include <vector>
#include <memory>
#include <DebugTools/DebugEntry/DebugEntry.h>
#include <Core/DirectX12/PostEffectExecutor.h>
#include <d3d12.h>
#include <Features/Cubemap/Skybox.h>
#include <drawable/base/DrawableBase.h>
#include <DebugTools/ImGuiManager/ImGuiManager.h>
#include <Interfaces/IGameEye.h>



/// <summary>
/// キャンバスクラス
/// </summary>
class Canvas : public DrawableBase
{
public:
    struct Params
    {
        std::string name = "Canvas";
        DirectX12* pDx12 = nullptr;
        CubemapSystem* pCubemapSystem = nullptr;
        IGameEye* pGameEye = nullptr;

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
    inline void SetEnableManualDraw(bool enable) { isEnableManualDraw_ = enable; }
    inline void SetGameEye(IGameEye* pGameEye) { params_.pGameEye = pGameEye; }

    /// <summary>
    /// 描画対象のスプライトを登録します
    /// </summary>
    /// <param name="sprite">登録するスプライト</param>
    /// <returns>自身への参照</returns>
    Canvas& RegisterDrawable(DrawableBase* drawable);

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
    inline bool IsEnableManualDraw() const { return isEnableManualDraw_; }
    [[nodiscard]]
    inline std::string GetName() const { return params_.name; }
    [[nodiscard]]
    inline const DX12Resource& GetResource() const { return resource_; }
    [[nodiscard]]
    inline PostEffectExecutor& GetPostEffectExecutor() const { return *pPostEffectExecutor_.get(); }
    [[nodiscard]]
    inline ID3D12GraphicsCommandList* GetCommandList() const
    {
        return pPostEffectExecutor_->GetCommandList();
    }
    [[nodiscard]]
    IGameEye* GetGameEye() const { return params_.pGameEye; }

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
    void DrawCall(ID3D12GraphicsCommandList* cl) override;
    
    /// <summary>
    /// デバッグ用 UI を描画します。
    /// </summary>
    void ImGui();

private:
    void ParameterCheck(const Canvas::Params& params) const;
    void ImGuiPreview();

    DX12Resource resource_ = {};
    bool isEnabled_ = true;
    bool isEnableManualDraw_ = false;

    Canvas::Params params_ = {};
    std::list<DrawableBase*> drawables_;
    std::list<Skybox*> skyboxes_;

    std::unique_ptr<DebugEntry<Canvas>> pDebugEntry_ = nullptr;
    std::unique_ptr<PostEffectExecutor> pPostEffectExecutor_ = nullptr;
};