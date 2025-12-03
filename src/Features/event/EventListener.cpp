#include "EventListener.h"

void EventListener::Dispatch()
{
    // 登録されているすべてのイベントIDについて処理を行う
    for (const auto& [eventID, publisherData] : eventQueue_)
    {
        // そのイベントIDに登録されているすべてのコールバック関数を呼び出す
        auto it = subscribers_.find(eventID);
        if (it == subscribers_.end()) continue;

        for (const auto& handler : it->second)
        {
            if (handler) handler(publisherData);
        }
    }

    // イベントキューをクリア
    eventQueue_.clear();
}

