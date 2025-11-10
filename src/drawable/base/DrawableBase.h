#pragma once

#include <d3d12.h>

class Canvas;

class DrawableBase
{
public:
    virtual ~DrawableBase();

    /// <summary>
    /// 1フレーム分の描画処理を行います。
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
    virtual void DrawCall() = 0;

protected:
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle_ = {};
    bool isDrawCalled_ = false;

private:
    Canvas* pCanvasCurrent_ = nullptr;
};