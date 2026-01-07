#pragma once
#include <features/screen/IntermediateScreenBase.h>
#include <memory>

class IIntermediateScreenFactory
{
public:
    virtual ~IIntermediateScreenFactory() {};

    /// <summary>
    /// 中間スクリーンを生成します。
    /// </summary>
    /// <returns>生成した中間スクリーンのポインタ。</returns>
    virtual std::unique_ptr<IntermediateScreenBase> Create(const std::string& name, ISceneArgs*) = 0;
};