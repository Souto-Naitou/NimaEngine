#include "./SceneTransitionExecutor.h"
#include <Features/SceneManager/SceneManager.h>

STEUpdateWithoutLoading::STEUpdateWithoutLoading() noexcept
{
    flags_ =
    {
        { Phase::PlayInAnimation,    false },
        { Phase::ReserveScene,       false },
        { Phase::PlayOutAnimation,   false },
        { Phase::End,                false },
    };
}

void STEUpdateWithoutLoading::operator()(SceneTransitionExecutor* pExecutor)
{
    auto& context = pExecutor->GetContext();
    auto& pTrans = context.pTransition_;
    bool isEnd = pTrans->IsEnd();

    /// フェードイン (まだフェードインを再生していない場合)
    if (!flags_[Phase::PlayInAnimation])
    {
        if (!pTrans->IsPlayed())
        {
            pTrans->PlayInAnimation();
            isEnd = false;
        }
        flags_[Phase::PlayInAnimation] = isEnd;
    }

    /// シーンチェンジ (フェードインが終了した後、まだシーン予約していない場合)
    else if (flags_[Phase::PlayInAnimation] && !flags_[Phase::ReserveScene])
    {
        SceneManager::GetInstance()->ReserveScene(context.nextSceneName_);
        flags_[Phase::ReserveScene] = true;
    }

    /// フェードアウト (シーン予約が済んだら)
    else if (flags_[Phase::ReserveScene] && !flags_[Phase::PlayOutAnimation])
    {
        if (!pTrans->IsPlayed())
        {
            pTrans->PlayOutAnimation();
            isEnd = false;
        }
        flags_[Phase::PlayOutAnimation] = isEnd;
    }

    /// 終了 (フェードアウトが終了したら)
    else if (flags_[Phase::PlayOutAnimation] && !flags_[Phase::End])
    {
        pTrans.reset();
        flags_[Phase::End] = true;
    }

    if (isEnd && pTrans->IsPlayed()) pTrans->Reset();
}

void STEUpdateWithoutLoading::Reset() noexcept
{
    for (auto& [phase, flag] : flags_)
    {
        flag = false;
    }
}