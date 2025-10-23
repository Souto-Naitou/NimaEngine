#pragma once

#include <Features/Model/IModel.h>
#include <Core/DirectX12/DirectX12.h>

#include <memory>
#include <string>


// モデルローダーインターフェイス
// - モデルをファイルから読み込む
class IModelLoader : public EngineFeature
{
public:
    virtual ~IModelLoader() = default;
    /// <summary>
    /// ローダーを初期化します。
    /// </summary>
    virtual void Initialize() = 0;
    /// <summary>
    /// 内部状態の更新を行います。
    /// </summary>
    virtual void Update() = 0;

    /// <summary>
    /// モデルを読み込みます。
    /// </summary>
    /// <param name="_path">モデルファイルのパス。</param>
    /// <returns>読み込んだモデルの共有ポインタ。</returns>
    virtual std::shared_ptr<IModel> LoadModel(const std::string& _path) = 0;
};