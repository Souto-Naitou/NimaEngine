#include "SceneBase.h"

SceneBase::SceneBase(ISceneArgs* _pArgs)
{
    pArgs_ = _pArgs;
    pLayer_ = std::any_cast<Layer*>(pArgs_->Get("Layer"));
}
