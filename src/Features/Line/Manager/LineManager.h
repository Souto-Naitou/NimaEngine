#pragma once

/// <summary>
/// ラインまとめ描画クラス
/// </summary>
class LineManager
{
public:
    LineManager(const LineManager&) = delete;
    LineManager& operator=(const LineManager&) = delete;
    LineManager(LineManager&&) = delete;
    LineManager& operator=(LineManager&&) = delete;

    static LineManager* GetInstance()
    {
        static LineManager instance;
        return &instance;
    }

    void Draw();

private:
    LineManager() = default;
    ~LineManager() = default;

};