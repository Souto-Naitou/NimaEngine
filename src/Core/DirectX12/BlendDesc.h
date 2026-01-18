#pragma once

#include <d3d12.h>

/// <summary>
/// ブレンドモード
/// </summary>
class BlendDesc
{
public:
    enum class BlendModes
    {
        None,       // !< ブレンドなし
        Normal,     // !< 通常αブレンド。デフォルト
        Alpha,      // !< 通常αブレンド。Normalと同じ
        Add,        // !< 加算
        Subtract,   // !< 減算
        Multiply,   // !< 乗算
        Screen,     // !< スクリーン
        Test,       // !< テスト用

        COUNT,      // !< カウント用
    };

    void Initialize(BlendModes mode);
    void ChangeMode(BlendModes mode);

    /// <summary>
    /// 現在のブレンドモードを取得します。
    /// </summary>
    /// <returns>現在のブレンドモード。</returns>
    [[nodiscard]] inline BlendModes GetBlendMode() const { return currentMode_; }

    /// <summary>
    /// ブレンド記述子を取得します。
    /// </summary>
    /// <returns>ブレンド記述子への定数参照。</returns>
    [[nodiscard]] inline const D3D12_BLEND_DESC& Get() const { return desc_; }

private:
    BlendModes currentMode_ = {};
    D3D12_BLEND_DESC desc_ = {};
};