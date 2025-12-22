#pragma once

#include <string>
#include <Features/Model/IModel.h>
#include <Features/Model/ModelManager.h>
#include <memory>
#include <Core/DirectX12/DirectX12.h>
#include <Features/Model/Loader/Assimp/ModelLoaderAssimp.h>
#include <Features/Model/ModelStorage.h>

struct ModelData;
struct MaterialData;

namespace Helper::Model
{
    ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename, const std::string& texturePath = {});
    MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename, const std::string& texturePath = {});
    void DispatchModel(IModel* pModel);

    template <typename _Loader>
    std::unique_ptr<_Loader> CreateLoader(DirectX12* pDx12)
    {
        // IModelLoaderを継承しているかどうか
        static_assert(std::is_base_of<IModelLoader, _Loader>::value, "Loader must inherit from IModelLoader");

        auto pLoader = std::make_unique<_Loader>();
        pLoader->SetDirectX12(pDx12);
        pLoader->Initialize();
        return pLoader;
    }
    std::unique_ptr<ModelStorage> CreateStorage();
    std::unique_ptr<ModelManager> CreateManager(IModelLoader* pLoader, ModelStorage* pStorage);
}