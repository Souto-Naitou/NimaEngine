#pragma once

#include <map>

/// <summary>
/// デルタタイム共有クラス
/// </summary>
class DeltaTimeManager
{
public:
    DeltaTimeManager(DeltaTimeManager const&) = delete;
    void operator=(DeltaTimeManager const&) = delete;
    DeltaTimeManager(DeltaTimeManager&&) = delete;
    DeltaTimeManager& operator=(DeltaTimeManager&&) = delete;

    static DeltaTimeManager* GetInstance()
    {
        static DeltaTimeManager instance;
        return &instance;
    }

    void SetDeltaTime(uint32_t _channel, float _deltaTime)
    {
        deltaTimes_[_channel] = _deltaTime;
    }

    float GetDeltaTime(uint32_t _channel) const
    {
        return deltaTimes_.at(_channel);
    }

private:
    DeltaTimeManager() = default;
    ~DeltaTimeManager() = default;


private:
    // DeltaTimeMap (channel, deltaTime)
    std::map<uint32_t, float> deltaTimes_;
};