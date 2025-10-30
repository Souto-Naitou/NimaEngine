#include "./ModelLoaderLegacy.h"
#include <Features/Model/ObjModel.h>
#include <Features/Model/Helper/ModelHelper.h>


void ModelLoaderLegacy::Initialize()
{
    pThreadPool_ = ThreadPool::GetInstance();
}

void ModelLoaderLegacy::Update()
{
    if (!uploadQueue_.empty())
    {
        auto frontModel = uploadQueue_.front();
        frontModel->CreateGPUResource();
        uploadQueue_.pop();
    }
}

std::shared_ptr<IModel> ModelLoaderLegacy::LoadModel(const std::string& _path)
{
    std::filesystem::path fsPath = _path;
    
    // OBJファイルでなければ例外を投げる
    if (fsPath.extension() != ".obj")
    {
        throw std::runtime_error("File is not the obj format");
    }

    std::shared_ptr<ObjModel> model = std::make_unique<ObjModel>();
    model->SetDirectX12(pDx12_);
    model->Initialize();

    auto loadObj = [model, this, _path]() -> void
    {
        *model->GetModelData() = Helper::Model::LoadObjFile({}, _path);
        this->EnqueueUpload(model.get());
    };

    pThreadPool_->enqueue(loadObj);

    return model;
}

void ModelLoaderLegacy::EnqueueUpload(IModel* _ptr)
{
    std::lock_guard<std::mutex> lock(mtx_);
    uploadQueue_.push(_ptr);
}
