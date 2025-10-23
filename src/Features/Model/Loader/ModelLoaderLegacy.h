#pragma once
#include "./IModelLoader.h"
#include <Features/Model/IModel.h>
#include <Features/ThreadPool/ThreadPool.h>

#include <mutex>
#include <memory>
#include <Core/DirectX12/DirectX12.h>
#include <queue>

// モデル読み込み機能クラス(旧実装)
// - ファイルからモデルを生成して返す
// - objファイルをマルチスレッドで読み込む
class ModelLoaderLegacy : public IModelLoader
{
public:
    void Initialize() override;
    void Update() override;
    std::shared_ptr<IModel> LoadModel(const std::string& _path) override;

private:
    /// アップロードキューに登録
    void EnqueueUpload(IModel* _ptr);

    mutable std::mutex mtx_;

    std::queue<IModel*> uploadQueue_;
    
    // Pointers
    ThreadPool* pThreadPool_ = nullptr;
};