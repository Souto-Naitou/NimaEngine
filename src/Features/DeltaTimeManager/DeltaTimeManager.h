#pragma once

#include <map>
#include <cstdint>

enum class DeltaTimeChannelReserved : uint32_t
{
    Default = 0,
    Game = 1,
    Particle = 2,
};

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

    inline void SetDeltaTime(DeltaTimeChannelReserved channel, float deltaTime)
    {
        this->SetDeltaTime(static_cast<uint32_t>(channel), deltaTime);
    }

    inline void SetDeltaTime(uint32_t channel, float deltaTime)
    {
        deltaTimes_[channel] = deltaTime;
    }

    float GetDeltaTime(uint32_t channel) const
    {
        return deltaTimes_.at(channel);
    }

private:
    DeltaTimeManager() = default;
    ~DeltaTimeManager() = default;


private:
    // DeltaTimeMap (channel, deltaTime)
    std::map<uint32_t, float> deltaTimes_;
};