#pragma once

#include <Core/DirectX12/DirectX12.h>
#include <DebugTools/DebugEntry/DebugEntry.h>

#include "TextSystem.h"
#include <memory>

enum class TextStandardPoint : int
{
    TopLeft,
    TopCenter,
    TopRight,
    CenterLeft,
    Center,
    CenterRight,
    BottomLeft,
    BottomCenter,
    BottomRight
};

class Text
{
public:
    Text() = default;
    ~Text() = default;
    
    /// <summary>
    /// テキスト描画に必要なリソースを初期化します。
    /// </summary>
    void Initialize();
    
    /// <summary>
    /// テキストの状態（位置・サイズなど）の更新を行います。
    /// </summary>
    void Update();
    
    /// <summary>
    /// テキストを描画します。
    /// </summary>
    void Draw();
    
    /// <summary>
    /// リソースの解放を行います。
    /// </summary>
    void Finalize();
    
    /// <summary>
    /// デバッグUIを描画します。
    /// </summary>
    void ImGui();

public: /// Getter
    Vector2 GetSize() const { return { metrics_.width, metrics_.height }; }


public: /// Setter
    void SetName(const std::string& name) { if(pDebugEntry_) pDebugEntry_->SetName(name); }
    /// <summary>
    /// 表示する文字列を設定します。
    /// </summary>
    /// <param name="text">文字列。</param>
    void SetText(const std::string& text);
    void SetAnchorPoint(TextStandardPoint anchor) { anchorPoint_ = anchor; UpdatePosition(); }
    void SetPivot(TextStandardPoint pivot) { pivot_ = pivot; UpdatePosition(); }
    
    /// <summary>
    /// 画面上の座標を設定します。
    /// </summary>
    /// <param name="pos">座標（ピクセル）。</param>
    void SetPosition(const Vector2& pos);
    void SetMaxSize(const Vector2& maxsize) { maxsize_ = maxsize; isChanged_ = true; }
    void SetFontFamily(const std::string& fontFamily) { fontFamily_ = fontFamily; isChanged_ = true; }
    void SetFontSize(float fontSize) { fontSize_ = fontSize; isChanged_ = true;}
    void SetColorName(const std::string& key) { keyColor_ = key; }
    /// <summary>
    /// 親テキストを設定します（相対位置計算に使用）。
    /// </summary>
    /// <param name="parent">親となる Text。</param>
    void SetParent(Text* parent);

    /// <summary>
    /// 現在の文字列・フォント設定からレイアウトを再生成します。
    /// </summary>
    void CreateTextLayout();


private: /// メンバー
    std::string                                 text_                       = {};
    std::string                                 fontFamily_                 = {};
    Vector2                                     screenPosition_             = {};
    Vector2                                     position_                   = {};
    TextStandardPoint                           anchorPoint_                = TextStandardPoint::TopLeft;
    TextStandardPoint                           pivot_                      = TextStandardPoint::TopLeft;
    Vector2                                     maxsize_                    = {};
    float                                       fontSize_                   = 0.0f;
    std::string                                 keyColor_                   = "Black";

    bool                                        isChanged_                  = false;
    bool                                        isChangedParent_            = false;
    bool                                        isVisibleRange_             = false;

    IDWriteTextFormat*                          textFormat_                 = nullptr;

    Text*                                       pParent_                    = nullptr;
    Text*                                       child_                      = nullptr;
    Microsoft::WRL::ComPtr<IDWriteTextLayout>   textLayout_                 = nullptr;
    DWRITE_TEXT_METRICS                         metrics_                    = {};

    std::unique_ptr<DebugEntry<Text>>           pDebugEntry_                = nullptr;

private: /// 借り物
    IDWriteFactory7*                            dwriteFactory_              = nullptr;
    TextSystem*                                 pTextSystem_                = nullptr;
    ID2D1DeviceContext2*                        d2dDeviceContext_           = nullptr;
    Viewport*                                   pViewport_                  = nullptr;


private: /// デバッグ
    const char*                                 anchors_[9]                 = { "TopLeft", "TopCenter", "TopRight", "CenterLeft", "Center", "CenterRight", "BottomLeft", "BottomCenter", "BottomRight" };
    int                                         callCount_UpdatePosition_   = 0;

private:
    /// <summary>
    /// 基準点から描画位置を算出します。
    /// </summary>
    /// <param name="stdpoint">基準点。</param>
    /// <returns>算出された座標。</returns>
    Vector2 ComputeStandardPosition(TextStandardPoint stdpoint);
    
    /// <summary>
    /// アンカー・ピボット・親子関係に基づき描画位置を更新します。
    /// </summary>
    void UpdatePosition();
    void SetChild(Text* child) { child_ = child; }
};