#include "EventSubscription.h"
#include "EventListener.h"

EventSubscription::~EventSubscription()
{
    if (owner_)
    {
        owner_->Unsubscribe(id_, index_);
    }
}

EventSubscription& EventSubscription::operator=(EventSubscription&& other) noexcept
{
    if (this != &other)
    {
        // 既存のサブスクリプションを解除
        if (owner_)
        {
            owner_->Unsubscribe(id_, index_);
        }
        id_ = other.id_;
        index_ = other.index_;
        owner_ = other.owner_;
        other.owner_ = nullptr;
    }
    return *this;
}
