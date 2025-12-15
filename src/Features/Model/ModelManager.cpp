#include "ModelManager.h"
#include <stdexcept>
#include <Utility/String/strutl.h>
#include <Core/ConfigManager/ConfigManager.h>

void ModelManager::Initialize()
{
    // Configに記述されているフォルダの追加
    auto& cfgData = ConfigManager::GetInstance()->GetConfigData();
    for (auto& path : cfgData.model_paths)
    {
        this->AddLoadPath(path);
        this->AddSearchPath(path);
    }
}

void ModelManager::SetModelLoader(IModelLoader* loader)
{
    if (loader != nullptr)
    {
        pModelLoader_ = loader;
    }
    else
    {
        throw std::invalid_argument("Model loader cannot be null");
    }
}

void ModelManager::SetModelStorage(ModelStorage* storage)
{
    if (storage != nullptr)
    {
        pModelStorage_ = storage;
    }
    else
    {
        throw std::invalid_argument("Model storage cannot be null");
    }
}

void ModelManager::AddLoadPath(const std::string& path)
{
    auto lowerPath = utl::string::to_lower(path);

    for (const auto& path : loadPaths_)
    {
        if (path == lowerPath)
        {
            // The path already exists, no need to add it again
            return;
        }
    }

    loadPaths_.push_back(lowerPath);
}

void ModelManager::AddSearchPath(const std::string& path)
{
    auto lowerPath = utl::string::to_lower(path);
    pathResolver_.AddSearchPath(lowerPath);
}

int ModelManager::LoadAll()
{
    int count = 0;
    for (const auto& path : loadPaths_)
    {
        for (auto itr = std::filesystem::recursive_directory_iterator(path, std::filesystem::directory_options::skip_permission_denied);
            itr != std::filesystem::recursive_directory_iterator();
            ++itr)
        {
            std::filesystem::path objPath = itr->path();
            if (objPath.extension() != ".obj")
            {
                continue;
            }
            this->Load(objPath.string());
            ++count;
        }
    }

    return count;
}

IModel* ModelManager::Load(const std::string& path)
{
    auto resolvedPath = pathResolver_.GetFilePath(path);
    if (pModelStorage_->IsLoaded(resolvedPath))
    {
        // If the model is already loaded, return it
        return pModelStorage_->FindModel(resolvedPath);
    }

    auto model = pModelLoader_->LoadModel(resolvedPath);

    if (model == nullptr)
    {
        throw std::runtime_error("Failed to load model from path: " + resolvedPath.empty() ? "(empty)"_s : resolvedPath);
    }

    return pModelStorage_->AddModel(resolvedPath, model);
}