#include "SceneBase.h"

SceneBase::SceneBase(ISceneArgs* pArgs)
{
    pArgs_ = pArgs;
    pLayer_ = std::any_cast<OrderedCanvasLayer*>(pArgs_->Get("Layer"));
}
