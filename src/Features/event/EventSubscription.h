#pragma once
#include <cstdint>

class EventListener;
using EventID = uint32_t;

/// <summary>
/// イベントサブスクリプションクラス
/// イベントのコールバック登録を自動で解除する (RAII前提)
/// </summary>
class EventSubscription
{
public:
    EventSubscription(EventID id, size_t index, EventListener* owner)
        : id_(id), index_(index), owner_(owner) {}

    ~EventSubscription();

    // コピー禁止
    EventSubscription(const EventSubscription&) = delete;
    EventSubscription& operator=(const EventSubscription&) = delete;
    // ムーブ許可
    EventSubscription(EventSubscription&& other) noexcept
        : id_(other.id_), index_(other.index_), owner_(other.owner_)
    {
        other.owner_ = nullptr;
    }

    EventSubscription& operator=(EventSubscription&& other) noexcept;

private:
    EventID id_;
    size_t index_;
    EventListener* owner_;
};
