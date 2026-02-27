#pragma once

#include <unordered_map>
#include <functional>
#include <any>
#include <vector>
#include <utility>
#include "./EventSubscription.h"

/// <summary>
/// イベント送受信クラス
/// </summary>
class EventListener
{
public:
    EventListener(EventListener&&) = delete;
    EventListener& operator=(EventListener&&) = delete;
    EventListener(const EventListener&) = delete;
    EventListener& operator=(const EventListener&) = delete;

    static EventListener* GetInstance()
    {
        static EventListener instance;
        return &instance;
    }

    template <typename T>
    inline void Publish(const T& data)
    {
        const EventID id = GetEventID<T>();
        eventQueue_.emplace_back(id, std::any(data));
    }

    template <typename T>
    EventSubscription Subscribe(std::function<void(const T&)> callback);

    /// EventSubscriptionのデストラクタから呼び出される、コールバックの解除関数
    inline void Unsubscribe(EventID id, size_t index)
    {
        auto it = subscribers_.find(id);
        if (it == subscribers_.end()) return;

        if (index < it->second.size())
        {
            it->second[index] = nullptr;
        }
    }

    /// <summary>
    /// 処理をディスパッチします。
    /// (イベントキューに溜まったイベントを、登録されたコールバックに対して順番に処理します)
    /// </summary>
    void Dispatch();

    template <typename T>
    static EventID GetEventID()
    {
        // 一度だけ初期化される静的変数を利用して、一意のIDを生成
        static const EventID id = nextEventID_++;
        return id;
    }

private:
    EventListener() = default;
    ~EventListener() = default;

private:
    using Handler = std::function<void(const std::any&)>;
    std::unordered_map<EventID, std::vector<Handler>> subscribers_;
    std::vector<std::pair<EventID, std::any>> eventQueue_;
    static inline EventID nextEventID_ = 0;
};

template <typename T>
EventSubscription EventListener::Subscribe(std::function<void(const T&)> callback)
{
    const EventID id = GetEventID<T>();
    size_t index = subscribers_[id].size();
    subscribers_[id].emplace_back([cb = std::move(callback)](const std::any& payload)
    {
        cb(std::any_cast<const T&>(payload));
    });

    return { id, index, this };
}
