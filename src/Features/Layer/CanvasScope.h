#pragma once

// 前方宣言を使用
class Canvas;

// CurrentCanvasのインクルードは必要（静的メソッドを使用するため）
#include "CurrentCanvas.h"

/// スコープを利用したキャンバスの切り替えクラス
/// 生成時に新しいキャンバスを設定し、破棄時に元のキャンバスに戻す
class CanvasScope
{
public:
    inline CanvasScope(Canvas* newCanvas)
    {
        previousCanvas_ = CurrentCanvas::Get();
        CurrentCanvas::Set(newCanvas);
    }
    inline ~CanvasScope()
    {
        CurrentCanvas::Set(previousCanvas_);
    }
private:
    Canvas* previousCanvas_ = nullptr;
};
