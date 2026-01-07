#include "./SceneTransitionExecutor.h"
#include "Features/SceneManager/SceneManager.h"

STEUpdateWithLoading::STEUpdateWithLoading() noexcept
{
    flags_ =
    {
        { Phase::PlayInAnimation0,      false },
        { Phase::CreateLoadingScreen,   false },
        { Phase::PlayOutAnimation0,     false },
        { Phase::PreloadNextScene,      false },
        { Phase::UpdateLoadingScreen,   false },
        { Phase::PlayInAnimation1,      false },
        { Phase::ReserveScene,          false },
        { Phase::PlayOutAnimation1,     false },
        { Phase::End,                   false },
    };
}

void STEUpdateWithLoading::operator()(SceneTransitionExecutor* pExecutor)
{
    auto& context = pExecutor->GetContext();
    auto& trans = context.pTransition_;
    bool isEnd = trans->IsEnd();
    auto& screen = context.pInterScreen_;

    /// フェードイン0
    if (IsEnablePhase(Phase::PlayInAnimation0))
    {
        if (!trans->IsPlayed())
        {
            trans->PlayInAnimation();
            isEnd = false;
        }
        flags_[Phase::PlayInAnimation0] = isEnd;
    }

    /// ローディングスクリーンの生成
    else if (IsEnablePhase(Phase::CreateLoadingScreen))
    {
        if (context.pInterScreenFactory_ && context.loadingScreenName_.has_value())
        {
            screen = context.pInterScreenFactory_->Create(context.loadingScreenName_.value(), context.pArg_);
        }
        flags_[Phase::CreateLoadingScreen] = true;
    }

    /// 次シーンのPreloading実行
    else if (IsEnablePhase(Phase::PreloadNextScene))
    {
        SceneManager::GetInstance()->ScenePreload(context.nextSceneName_, context.pInterScreen_->GetTaskExecutor());
        screen->Initialize();
        context.isScreenInitialized_ = true;
        flags_[Phase::PreloadNextScene] = true;
    }

    /// フェードアウト0
    else if (IsEnablePhase(Phase::PlayOutAnimation0))
    {
        if (!trans->IsPlayed())
        {
            trans->PlayOutAnimation();
            isEnd = false;
        }
        flags_[Phase::PlayOutAnimation0] = isEnd;
    }

    /// ローディングスクリーンの更新完了待ち
    else if (IsEnablePhase(Phase::UpdateLoadingScreen))
    {
        if (screen && screen->IsEnd())
        {
            flags_[Phase::UpdateLoadingScreen] = true;
        }
    }

    /// フェードイン1
    else if (IsEnablePhase(Phase::PlayInAnimation1))
    {
        if (!trans->IsPlayed())
        {
            trans->PlayInAnimation();
            isEnd = false;
        }
        flags_[Phase::PlayInAnimation1] = isEnd;
    }

    /// シーン予約
    else if (IsEnablePhase(Phase::ReserveScene))
    {
        screen->Finalize();
        screen.reset();
        SceneManager::GetInstance()->ReserveScene(context.nextSceneName_);
        flags_[Phase::ReserveScene] = true;
    }

    /// フェードアウト1
    else if (IsEnablePhase(Phase::PlayOutAnimation1))
    {
        if (!trans->IsPlayed())
        {
            trans->PlayOutAnimation();
            isEnd = false;
        }
        flags_[Phase::PlayOutAnimation1] = isEnd;
    }

    /// 終了
    else if (IsEnablePhase(Phase::End))
    {
        trans.reset();
        flags_[Phase::End] = true;
    }

    if (isEnd && trans->IsPlayed()) trans->Reset();

    if (screen && context.isScreenInitialized_)
    {
        screen->Update();
    }
}

void STEUpdateWithLoading::Reset() noexcept
{
    for (auto& [phase, flag] : flags_)
    {
        flag = false;
    }
}

bool STEUpdateWithLoading::IsEnablePhase(Phase phase)
{
    auto curr = flags_.find(phase);

    if (curr == flags_.begin())
    {
        return !curr->second;
    }

    auto prev = std::prev(curr);
    return prev->second && !curr->second;
}
