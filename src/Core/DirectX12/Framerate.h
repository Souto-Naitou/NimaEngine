#pragma once

#include <chrono>
#include <Features/TimeMeasurer/HiResoStopWatch.h>

/// <summary>
/// フレームレート管理クラス
/// </summary>
class FrameRate
{
public:
    FrameRate(const FrameRate&) = delete;
    FrameRate(const FrameRate&&) = delete;
    FrameRate& operator=(const FrameRate&) = delete;
    FrameRate& operator=(const FrameRate&&) = delete;

    static FrameRate* GetInstance() { static FrameRate instance; return &instance; }

    /// <summary>
    /// フレームレート管理を初期化します。
    /// </summary>
    void    Initialize();
    /// <summary>
    /// ターゲットフレームレートに合わせてスリープを行います。
    /// </summary>
    void    FixFramerate();
    /// <summary>
    /// 一定間隔で FPS を測定します。
    /// </summary>
    void    MeasureFPS();

    void    SetIntervalCalculationFPS(double interval) { intervalCalculationFPS_ = interval; }
    double  GetFPS() const { return fps_; }

    /// <summary>
    /// フレームレート管理の有効/無効を設定します。
    /// </summary>
    /// <param name="flag"></param>
    void    Enable(bool flag) { enable_ = flag; }
    bool    IsEnable() const { return enable_; }

private:
    FrameRate() = default;
    ~FrameRate() = default;
    std::chrono::steady_clock::time_point reference_;

    HiResoStopWatch timer_ = {};

    bool            enable_                 = true;

    const double    kTargetFPS_             = 60.0;
    double          fps_                    = 0.0;
    unsigned int    frameCount_             = 0u;
    double          windowStartTime_        = 0.0;
    double          intervalCalculationFPS_ = 2.0;
};