#include "SceneBase.h"

SceneBase::SceneBase(ISceneArgs* pArgs)
{
    pArgs_ = pArgs;
    pLayer_ = std::any_cast<Layer*>(pArgs_->Get("Layer"));
}
