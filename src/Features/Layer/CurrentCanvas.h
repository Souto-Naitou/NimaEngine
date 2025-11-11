#pragma once

// 前方宣言を使用してインクルードを最小化
class Canvas;

class CurrentCanvas
{
public:
    inline static Canvas* Get()
    {
        return currentCanvas_;
    }

    inline static void Set(Canvas* canvas)
    {
        currentCanvas_ = canvas;
  }

private:
    inline static Canvas* currentCanvas_ = nullptr;
};