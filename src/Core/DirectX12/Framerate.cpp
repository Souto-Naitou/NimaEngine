#include "Framerate.h"

#include <thread>
#include <timeapi.h>

#pragma comment(lib, "Winmm.lib")

void FrameRate::Initialize()
{
    timeBeginPeriod(1);
    reference_ = std::chrono::steady_clock::now();
}

void FrameRate::FixFramerate()
{
    if (!enable_) return;

    const auto kFrameDuration =
        std::chrono::duration_cast<std::chrono::steady_clock::duration>(
            std::chrono::duration<double>(1.0 / kTargetFPS_));

    const auto target = reference_ + kFrameDuration;
    const auto now = std::chrono::steady_clock::now();

    // 目標時間を過ぎている場合は、次のフレームの基準時間を更新して終了
    if (now >= target)
    {
        reference_ = now;
        return;
    }

    constexpr auto kSpinMargin = std::chrono::milliseconds(1);

    if (target - now > kSpinMargin)
    {
        std::this_thread::sleep_until(target - kSpinMargin);
    }

    while (std::chrono::steady_clock::now() < target)
    {
        std::this_thread::yield();
    }

    reference_ = target;
}

void FrameRate::MeasureFPS()
{
    if (!timer_.GetIsStart())
    {
        timer_.Start();
    }

    ++frameCount_;

    auto    now     = timer_.GetNow<double>();
    double  elapsed = now - windowStartTime_;

    /// フレームレート計算
    if (elapsed >= intervalCalculationFPS_)
    {
        fps_             = frameCount_ / elapsed;
        frameCount_      = 0;
        windowStartTime_ = now;
    }
}
