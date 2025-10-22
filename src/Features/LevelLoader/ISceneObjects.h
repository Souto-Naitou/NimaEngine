#pragma once

#include <Features/Model/IModel.h>
#include <d3d12.h>

/// <summary>
/// シーンオブジェクトインターフェース
/// </summary>
class ISceneObjects
{
public:
    virtual ~ISceneObjects() = default;
    virtual void Initialize() = 0;
    virtual void Finalize() = 0;
    virtual void Update() = 0;
    virtual void Draw() = 0;
    virtual std::string GetName() const = 0;
};