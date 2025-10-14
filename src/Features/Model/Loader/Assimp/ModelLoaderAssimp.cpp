#include "ModelLoaderAssimp.h"

#include <Utility/FileSystem/fsutl.h>

#include <Core/DirectX12/Helper/DX12Helper.h>

#include <Assimp/Importer.hpp>
#include <Assimp/postprocess.h>
#include <Features/Model/ObjModel.h>
#include <Features/Model/GltfModel.h>
#include <utility>
#include <Core/DirectX12/SRVManager.h>
#include <algorithm>

void ModelLoaderAssimp::Initialize()
{
}

void ModelLoaderAssimp::Update()
{
}

std::shared_ptr<IModel> ModelLoaderAssimp::LoadModel(const std::string& _path)
{
    std::filesystem::path fsPath = _path;

    std::shared_ptr<IModel> model = nullptr;
    if (fsPath.extension() == ".obj")
    {
        auto objModel = std::make_shared<ObjModel>();
        objModel->SetDirectX12(pDx12_);
        objModel->Initialize();

        *objModel->GetModelData() = this->_LoadModelByAssimp(_path);
        objModel->CreateGPUResource();
        model = objModel;
    }
    else if (fsPath.extension() == ".gltf")
    {
        auto gltfModel = std::make_shared<GltfModel>();
        gltfModel->SetDirectX12(pDx12_);
        gltfModel->Initialize();

        *gltfModel->GetModelData() = this->_LoadModelByAssimp(_path);
        *gltfModel->GetAnimationData() = this->_LoadAnimation(_path);
        *gltfModel->GetSkeleton() = this->_CreateSkeleton(gltfModel->GetModelData()->rootNode);
        *gltfModel->GetSkinCluster() = this->_CreateSkinCluster(
            pDx12_->GetDevice(),
            *gltfModel->GetSkeleton(),
            *gltfModel->GetModelData()
        );
        gltfModel->CreateGPUResource();
        model = gltfModel;
    }

    return model;
}

ModelData ModelLoaderAssimp::_LoadModelByAssimp(const std::string& _path)
{
    ModelData result = {};
    Assimp::Importer importer;
    std::string parentPath = utl::filesystem::get_parent_path_string(_path);

    // 三角形の並びを反転させ、UV座標を反転させるフラグを設定
    uint32_t flags = aiProcess_FlipWindingOrder | aiProcess_FlipUVs | aiProcess_Triangulate | aiProcess_FixInfacingNormals ;

    const aiScene* scene = importer.ReadFile(_path.c_str(), flags);

    if (!scene->HasMeshes()) 
    {
        throw std::runtime_error("Failed to load model: " + _path);
    }

    // Meshの解析
    for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
    {
        const aiMesh* mesh = scene->mMeshes[meshIndex];
        if (!mesh->HasNormals()) [[unlikely]]
        {
            throw std::runtime_error("Mesh does not have normals: " + _path);
        }
        //if (!mesh->HasTextureCoords(0)) [[unlikely]]
        //{
        //    throw std::runtime_error("Mesh does not have texture coordinates: " + _path);
        //}

        result.vertices.reserve(mesh->mNumVertices);

        // Face(面)の処理
        for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex)
        {
            const aiFace& face = mesh->mFaces[faceIndex];
            // 三角面でない場合は例外を投げる(Triangulateが有効なので、通常は発生しないはず)
            if (face.mNumIndices != 3) [[unlikely]]
            {
                throw std::runtime_error("Mesh face is not a triangle: " + _path);
            }

            // インデックスの追加
            for (uint32_t element = 0; element < face.mNumIndices; ++element)
            {
                uint32_t vertexIndex = face.mIndices[element];
                result.indices.push_back(vertexIndex);
            }
        }

        // 頂点データの追加
        for (uint32_t vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex)
        {
            const aiVector3D& position = mesh->mVertices[vertexIndex];
            const aiVector3D& normal = mesh->mNormals[vertexIndex];
            aiVector3D texcoord = {};

            if (mesh->HasTextureCoords(0))
            {
                texcoord = mesh->mTextureCoords[0][vertexIndex];
            }
            else
            {
                // テクスチャ座標がない場合はデフォルト値を設定
                texcoord = aiVector3D(0.0f, 0.0f, 0.0f);
            }

            VertexData vertex;
            vertex.position = Vector4(position.x, position.y, position.z, 1.0f);
            vertex.normal = Vector3(normal.x, normal.y, normal.z);
            vertex.texcoord = Vector2(texcoord.x, texcoord.y);
            // aiProcess_MakeLeftHandedが有効な場合、X軸を反転
            vertex.position.x *= -1.0f;
            vertex.normal.x *= -1.0f;
            result.vertices.push_back(vertex);
        }
        
        for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
        {
            // Jointごとの格納領域をつくる
            aiBone* bone = mesh->mBones[boneIndex];
            std::string jointName = bone->mName.C_Str();
            JointWeightData& jointWeightData = result.skinClusterData[jointName];

            aiMatrix4x4 bindPoseMatrixAssimp = bone->mOffsetMatrix.Inverse();
            aiVector3D scale, translate;
            aiQuaternion rotate;
            bindPoseMatrixAssimp.Decompose(scale, rotate, translate);
            // Assimpの座標系は右手系なので左手系に変換する
            Matrix4x4 bindPoseMatrix = Matrix4x4::AffineMatrix(
                Vector3{scale.x, scale.y, scale.z},
                Quaternion{rotate.x, -rotate.y, -rotate.z, rotate.w},
                Vector3{-translate.x, translate.y, translate.z}
            );
            jointWeightData.inverseBindPoseMatrix = bindPoseMatrix.Inverse();

            for (uint32_t weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex)
            {
                jointWeightData.vertexWeights.push_back(
                    { bone->mWeights[weightIndex].mWeight, bone->mWeights[weightIndex].mVertexId }
                );
            }
        }
    }

    for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex)
    {
        const aiMaterial* material = scene->mMaterials[materialIndex];
        if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
        {
            aiString texturePath;
            material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath);
            result.material.textureFilePath = parentPath + "/" + texturePath.C_Str();
        }
    }

    result.rootNode = this->_ReadNode(scene->mRootNode);

    return result;
}

Animation ModelLoaderAssimp::_LoadAnimation(const std::string& _path)
{
    Animation result = {};
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(_path.c_str(), 0);
    return this->_LoadAnimation(scene);
}

Animation ModelLoaderAssimp::_LoadAnimation(const aiScene* _scene)
{
    Animation result = {};

    if (_scene->mNumAnimations == 0) [[unlikely]]
    {
        throw std::runtime_error("No animations found in the scene.");
    }

    // TODO: 複数のアニメーションに対応する
    aiAnimation* animationAssimp = _scene->mAnimations[0];

    // アニメーションの情報を取り出す
    const float kAiDuration = static_cast<float>(animationAssimp->mDuration);
    const float kAiTicksPerSecond = static_cast<float>(animationAssimp->mTicksPerSecond);

    if (kAiTicksPerSecond == 0.0f) [[unlikely]]
    {
        throw std::runtime_error("Animation ticks per second is zero.");
        return result;
    }

    // アニメーションの長さ(秒単位)を計算
    result.duration = kAiDuration / kAiTicksPerSecond;

    // <ノードごとのアニメーションデータを取得>
    // NOTE: Assimpでは個々のNodeのAnimationをchannelと呼んでいる
    const uint32_t numChannels = animationAssimp->mNumChannels;

    // HACK: 抽象化して関数にまとめたい
    for (uint32_t channelIndex = 0; channelIndex < numChannels; ++channelIndex)
    {
        aiNodeAnim* nodeAnimationAssimp = animationAssimp->mChannels[channelIndex];
        // 書き込み先のNodeAnimationを(取得|作成)
        NodeAnimation& nodeAnimation = result.nodeAnimations[nodeAnimationAssimp->mNodeName.C_Str()];

        for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumPositionKeys; ++keyIndex)
        {
            const aiVectorKey& keyAssimp = nodeAnimationAssimp->mPositionKeys[keyIndex];
            Keyframe<Vector3> keyframe
            {
                .time = static_cast<float>(keyAssimp.mTime) / kAiTicksPerSecond,
                .value = Vector3(-keyAssimp.mValue.x, keyAssimp.mValue.y, keyAssimp.mValue.z)
            };
            nodeAnimation.translate.keyframes.push_back(keyframe);
        }

        for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumRotationKeys; ++keyIndex)
        {
            const aiQuatKey& keyAssimp = nodeAnimationAssimp->mRotationKeys[keyIndex];
            Keyframe<Quaternion> keyframe
            {
                .time = static_cast<float>(keyAssimp.mTime) / kAiTicksPerSecond,
                .value = Quaternion(keyAssimp.mValue.x, -keyAssimp.mValue.y, -keyAssimp.mValue.z, keyAssimp.mValue.w)
            };
            nodeAnimation.rotation.keyframes.push_back(keyframe);
        }

        for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumScalingKeys; ++keyIndex)
        {
            const aiVectorKey& keyAssimp = nodeAnimationAssimp->mScalingKeys[keyIndex];
            Keyframe<Vector3> keyframe
            {
                .time = static_cast<float>(keyAssimp.mTime) / kAiTicksPerSecond,
                .value = Vector3(keyAssimp.mValue.x, keyAssimp.mValue.y, keyAssimp.mValue.z)
            };
            nodeAnimation.scale.keyframes.push_back(keyframe);
        }
    }

    return result;
}

Node ModelLoaderAssimp::_ReadNode(aiNode* _node)
{
    Node node;

    aiVector3D scale, translate;
    aiQuaternion rotate;

    _node->mTransformation.Decompose(scale, rotate, translate);
    node.transform.scale = Vector3(scale.x, scale.y, scale.z);
    node.transform.rotate = Quaternion(rotate.x, -rotate.y, -rotate.z, rotate.w);
    node.transform.translate = Vector3(-translate.x, translate.y, translate.z);
    node.localMatrix = Matrix4x4::AffineMatrix(
        node.transform.scale,
        node.transform.rotate,
        node.transform.translate
    );

    node.name = _node->mName.C_Str();
    node.children.resize(_node->mNumChildren);
    for (uint32_t childIndex = 0; childIndex < _node->mNumChildren; ++childIndex)
    {
        node.children[childIndex] = _ReadNode(_node->mChildren[childIndex]);
    }

    return node;
}

Skeleton ModelLoaderAssimp::_CreateSkeleton(const Node& _rootNode)
{
    Skeleton skeleton;
    skeleton.Initialize();
    SkeletonData& skeletonData = skeleton.GetSkeletonData();
    skeletonData.rootIndex = _CreateJoint(_rootNode, {}, skeletonData.joints);

    // 名前とindexのマッピングを行いアクセスしやすくする
    for (const Joint& joint : skeletonData.joints)
    {
        const JointData& jointData = joint.GetJointData();
        skeletonData.jointMap.emplace(jointData.name, jointData.index);
    }

    for (Joint& joint : skeletonData.joints)
    {
        auto& jointData = joint.GetJointData();
        jointData.localMatrix = Matrix4x4::AffineMatrix(
            jointData.transform.scale,
            jointData.transform.rotate,
            jointData.transform.translate
        );
        if (jointData.parentIndex)
        {
            // 親ジョイントが存在するなら親の行列を掛ける
            const Matrix4x4& parentSkeletonSpaceMatrix = skeletonData.joints[*jointData.parentIndex].GetJointData().skeletonSpaceMatrix;
            jointData.skeletonSpaceMatrix = jointData.localMatrix * parentSkeletonSpaceMatrix;
        }
        else
        {
            // 親ジョイントがいないのでlocalMatrixとskeletonSpaceMatrixは同じ
            jointData.skeletonSpaceMatrix = jointData.localMatrix;
        }
    }

    return skeleton;
}

int32_t ModelLoaderAssimp::_CreateJoint(const Node& _node, const std::optional<int32_t>& _parentIndex, std::vector<Joint>& _joints)
{
    Joint joint;
    joint.Initialize();
    JointData& jointData = joint.GetJointData();
    jointData.name = _node.name;
    jointData.localMatrix = _node.localMatrix;
    jointData.skeletonSpaceMatrix = Matrix4x4::Identity();
    jointData.transform = _node.transform;
    jointData.index = static_cast<int32_t>(_joints.size());
    jointData.parentIndex = _parentIndex;
    _joints.push_back(joint);
    for (const Node& child : _node.children)
    {
        // 子Jointを作成してそのIndexを登録
        int32_t childIndex = this->_CreateJoint(child, jointData.index, _joints);
        _joints[jointData.index].GetJointData().childrenIndices.push_back(childIndex);
    }
    return jointData.index;
}

SkinCluster ModelLoaderAssimp::_CreateSkinCluster(
    const Microsoft::WRL::ComPtr<ID3D12Device>& _device,
    const Skeleton& _skeleton, 
    const ModelData& _modelData
)
{
    SkinCluster skinCluster;

    auto& skeletonData = _skeleton.GetSkeletonData();

    // Palette用のResourceを作成
    skinCluster.resourcePalette = DX12Helper::CreateBufferResource(
        _device,
        sizeof(WellForGPU) * _skeleton.GetSkeletonData().joints.size()
    );
    WellForGPU* mappedPalette = nullptr;
    skinCluster.resourcePalette->Map(0, nullptr, reinterpret_cast<void**>(&mappedPalette));
    skinCluster.mappedPalette = { mappedPalette, _skeleton.GetSkeletonData().joints.size() };
    
    // SRVを割り当てる
    auto* sm = SRVManager::GetInstance();
    skinCluster.srvIndexPalette = sm->Allocate();
    skinCluster.srvHandlePalette.first = sm->GetCPUDescriptorHandle(skinCluster.srvIndexPalette);
    skinCluster.srvHandlePalette.second = sm->GetGPUDescriptorHandle(skinCluster.srvIndexPalette);

    // palette用のSRVを作成 StructuredBufferとして作成
    sm->CreateForStructuredBuffer(
        skinCluster.srvIndexPalette,
        skinCluster.resourcePalette.Get(),
        static_cast<uint32_t>(skeletonData.joints.size()),
        sizeof(WellForGPU)
    );

    // インフルエンス用のResourceを作成
    skinCluster.resourceInfluence = DX12Helper::CreateBufferResource(
        _device,
        sizeof(VertexInfluence) * _modelData.vertices.size()
    );
    VertexInfluence* mappedInfluences = nullptr;
    skinCluster.resourceInfluence->Map(0, nullptr, reinterpret_cast<void**>(&mappedInfluences));
    std::memset(mappedInfluences, 0, sizeof(VertexInfluence) * _modelData.vertices.size());
    skinCluster.mappedInfluences = { mappedInfluences, _modelData.vertices.size() };

    // SRVを割り当てる
    skinCluster.srvIndexInfluence = sm->Allocate();
    skinCluster.srvHandleInfluence.first = sm->GetCPUDescriptorHandle(skinCluster.srvIndexInfluence);
    skinCluster.srvHandleInfluence.second = sm->GetGPUDescriptorHandle(skinCluster.srvIndexInfluence);
    
    // influence用のSRVを作成 StructuredBufferとして作成
    sm->CreateForStructuredBuffer(
        skinCluster.srvIndexInfluence,
        skinCluster.resourceInfluence.Get(),
        static_cast<uint32_t>(_modelData.vertices.size()),
        sizeof(VertexInfluence)
    );

    // SkinningInformation用のResourceを作成
    skinCluster.resourceSkinningInformation = DX12Helper::CreateBufferResource(
        _device,
        sizeof(SkinningInformation)
    );
    SkinningInformation* mappedSkinningInformation = nullptr;
    skinCluster.resourceSkinningInformation->Map(0, nullptr, reinterpret_cast<void**>(&mappedSkinningInformation));
    mappedSkinningInformation->numVertices = static_cast<uint32_t>(_modelData.vertices.size());
    skinCluster.mappedSkinningInformation = { mappedSkinningInformation, 1 };

    // SRVを割り当てる
    skinCluster.srvIndexSkinningInformation = sm->Allocate();

    // InverseBindPoseMatrixを格納する場所を作成して、単位行列で埋める
    auto& ibpm = skinCluster.inverseBindPoseMatrices;
    ibpm.resize(skeletonData.joints.size());
    std::generate(ibpm.begin(), ibpm.end(), Matrix4x4::Identity);
    
    for (const auto& jointWeight : _modelData.skinClusterData)
    {
        auto it = skeletonData.jointMap.find(jointWeight.first);
        if (it == skeletonData.jointMap.end())
        {
            // 名前が見つからない場合はネクスト！
            continue;
        }

        // (*it)secondにはjointのindexが入っているので、該当のindexのinverseBindPoseMatrixを代入
        ibpm[(*it).second] = jointWeight.second.inverseBindPoseMatrix;
        for (const auto& vertexWeight : jointWeight.second.vertexWeights)
        {
            auto& currentInfluence = skinCluster.mappedInfluences[vertexWeight.vertexIndex];
            for (uint32_t index = 0; index < kNumMaxInfluence; ++index)
            {
                if (currentInfluence.weights[index] == 0.0f)
                {
                    // 空いているところに値をセット
                    currentInfluence.weights[index] = vertexWeight.weight;
                    currentInfluence.jointIndices[index] = (*it).second;
                    break;
                }
            }
        }
    }

    return skinCluster;
}
