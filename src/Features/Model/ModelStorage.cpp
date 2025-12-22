#include "./ModelStorage.h"

#include <Utility/FileSystem/fsutl.h>


void ModelStorage::Finalize()
{
    for (auto& model : models_)
    {
        model.second->Finalize();
    }
}

IModel* ModelStorage::FindModel(const std::string& path)
{
    auto findPathLower = utl::filesystem::to_lower(path);
    auto findPathLowerAbs = std::filesystem::absolute(findPathLower);

    IModel* result = nullptr;

    for (auto& model : models_)
    {
        auto modelPathLower = utl::filesystem::to_lower(model.first);
        auto modelPathLowerAbs = std::filesystem::absolute(modelPathLower);

        if (modelPathLowerAbs == findPathLowerAbs)
        {
            result = model.second.get();
            break;
        }
    }

    return result;
}

bool ModelStorage::IsLoaded(const std::filesystem::path& path)
{
    for (auto& model : models_)
    {
        std::filesystem::path fsModelPath = utl::filesystem::to_lower(model.first);
        std::filesystem::path fsFullPath = utl::filesystem::to_lower(path);

        if (fsModelPath == fsFullPath)
        {
            return true;
        }
    }

    return false;
}

IModel* ModelStorage::AddModel(const std::filesystem::path& path, std::shared_ptr<IModel> model)
{
    std::filesystem::path fsPath = utl::filesystem::to_lower(path);
    if (this->IsLoaded(fsPath))
    {
        // すでに登録されていたら登録済みを返す
        return models_.at(fsPath).get();
    }
    auto itrPair =  models_.emplace(fsPath, model);
    return itrPair.first->second.get();
}
