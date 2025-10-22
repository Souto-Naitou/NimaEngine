#pragma once

#include <Features/TimeMeasurer/TimeMeasurer.h>
#include <string>
#include <unordered_map>
#include <functional>

/// <summary>
/// 実行時間計測クラス
/// </summary>
class EventTimer
{
public:
    EventTimer(const EventTimer&) = delete;
    EventTimer& operator=(const EventTimer&) = delete;
    EventTimer(const EventTimer&&) = delete;
    EventTimer& operator=(const EventTimer&&) = delete;

    static EventTimer* GetInstance()
    {
        static EventTimer instance;
        return &instance;
    }

    void NewFrame();
    void BeginEvent(const std::string& _eventName);
    void EndEvent(const std::string& _eventName);
    void Reset();

    void Measure(const std::string& _eventName, const std::function<void()>& _func);

    void EndFrame();

    void ImGui();

private:
    EventTimer() = default;
    ~EventTimer() = default;

private:
    TimeMeasurer globalTimer_;
    float deltaTime_ = 0.0f;
    int durationMaxTimeDisplay_ = 3;
    std::unordered_map<std::string, float> events_;
    std::unordered_map<std::string, TimeMeasurer> timers_;
    std::unordered_map<std::string, float> maxTime_;
    std::unordered_map<std::string, TimeMeasurer> maxTimeResetTimers_;
    bool isDisplayProgressBar_ = true;
};