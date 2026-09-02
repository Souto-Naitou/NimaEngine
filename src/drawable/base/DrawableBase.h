#pragma once

#include <d3d12.h>

class Canvas;

/// <summary>
/// 描画可能オブジェクトの基底クラス
/// </summary>
class DrawableBase
{
public:
    virtual ~DrawableBase();

    /// <summary>
    /// 呼び出されたフレームのみ描画します。
    /// </summary>
    virtual void Draw1F();

    /// <summary>
    /// レンダーターゲットビューのハンドルを設定します。
    /// </summary>
    /// <param name="rtvHandle">RTVハンドル</param>
    virtual void SetRTVHandle(const D3D12_CPU_DESCRIPTOR_HANDLE& rtvHandle) { rtvHandle_ = rtvHandle; }

    /// <summary>
    /// DrawCallをリストに積みます。
    /// </summary>
    virtual void DrawCall(ID3D12GraphicsCommandList* cl) = 0;

    /// <summary>
    /// キャンバスを切り離します。
    /// </summary>
    void DetachCurrentCanvas() { pCanvasCurrent_ = nullptr; }

    /// <summary>
    /// Draw1Fが呼ばれていない状態にリセットします。
    /// </summary>
    void ResetDraw1FCalled() { isDraw1FCalled_ = false; }

    bool IsDraw1FCalled() const { return isDraw1FCalled_; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetRTVHandleCPU() const { return rtvHandle_; }

    /// <summary>
    /// 現在のキャンバスを取得します。
    /// </summary>
    /// <returns></returns>
    Canvas* GetCurrentCanvas() const { return pCanvasCurrent_; }

private:
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle_ = {};
    bool isDraw1FCalled_ = false;
    Canvas* pCanvasCurrent_ = nullptr;
};