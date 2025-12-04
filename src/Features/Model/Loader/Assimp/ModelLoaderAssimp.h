#pragma once

#include <Features/Model/Loader/IModelLoader.h>
#include <Features/Model/ModelData.h>
#include <Features/Model/Skeleton.h>
#include <string>
#include <Assimp/scene.h>
#include <memory>
#include <Features/Model/IModel.h>
#include <Features/Model/Animation.h>
#include <optional>
#include <vector>
#include <Features/Model/SkinCluster.h>

// モデル読み込みクラス
// - Assimpを使用してモデルを読み込む
// - OBJファイル形式以外も対応可能
class ModelLoaderAssimp : public IModelLoader
{
public:
    void Initialize() override;
    void Update() override;
    std::shared_ptr<IModel> LoadModel(const std::string& _path) override;

private:
    ModelData   LoadModelByAssimp(const std::string& _path);
    // アニメーションの読み込み
    Animation   LoadAnimation(const aiScene* _scene);
    // ハンドラ
    Animation   LoadAnimation(const std::string& _path);
    // ノードの読み込み
    Node        ReadNode(aiNode* _node);

    // スケルトンの作成
    Skeleton    CreateSkeleton(const Node& _rootNode);

    int32_t     CreateJoint(const Node& _node, const std::optional<int32_t>& _parentIndex, std::vector<Joint>& _joints);

    SkinCluster CreateSkinCluster(
        const Microsoft::WRL::ComPtr<ID3D12Device>& _device,
        const Skeleton& _skeleton,
        const ModelData& _modelData
    );
};