#include "ModelHelper.h"

#include <Features/Model/ModelData.h>
#include <Vector4.h>
#include <Vector3.h>
#include <Vector2.h>

#include <fstream>
#include <sstream>
#include <cassert>
#include <filesystem>
#include <Features/Model/GltfModel.h>

ModelData Helper::Model::LoadObjFile(const std::string& dirPath, const std::string& fname, const std::string& texturePath)
{
    // 1 Decleare variable
    ModelData modelData;
    std::vector<Vector4>    positions;
    std::vector<Vector3>    normals;
    std::vector<Vector2>    texcoords;
    std::string             line;
    // 2 Open file
    std::string allPath;
    std::string directoryPath;
    std::string filename;
    if (dirPath.empty())
    {
        allPath = fname;
    }
    else
    {
        allPath = dirPath + "/" + fname;
    }
    std::filesystem::path filePath = allPath;
    directoryPath = filePath.parent_path().string();
    filename = filePath.filename().string();

    std::ifstream file(allPath);
    if (!file)
    {
        auto current = std::filesystem::current_path();
        std::string text = "Current path -> " + current.string() + "\n";

        OutputDebugStringA(text.c_str());
        OutputDebugStringA(std::string("Missing file : " + allPath + "\n").c_str());
        assert(file.is_open());
    }
    OutputDebugStringA(("\tCurrentPath : " + std::filesystem::current_path().string() + '\n').c_str());
    // 3 Read file and construct ModelData
    while (std::getline(file, line))
    {
        std::string identifier;
        std::istringstream s(line);
        s >> identifier;
        if (identifier == "#")
        {
            continue;
        }
        else if (identifier == "v")
        {
            Vector4 position;
            s >> position.x >> position.y >> position.z;
            position.x *= -1.0f;
            position.w = 1.0f;
            positions.push_back(position);
        }
        else if (identifier == "vt")
        {
            Vector2 texcoord;
            s >> texcoord.x >> texcoord.y;
            texcoord.y *= -1.0f;
            texcoords.push_back(texcoord);
        }
        else if (identifier == "vn")
        {
            Vector3 normal;
            s >> normal.x >> normal.y >> normal.z;
            normal.x *= -1.0f;
            normals.push_back(normal);
        }
        else if (identifier == "f")
        {
            VertexData triangle[3];
            // 面は三角形限定。その他は未対応
            for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex)
            {
                std::string vertexDefinition;
                s >> vertexDefinition;
                // 頂点の要素へのIndexは「位置 / UV / 法線」で格納される
                std::istringstream v(vertexDefinition);
                uint32_t elementIndices[3];
                for (int32_t element = 0; element < 3; ++element)
                {
                    std::string index;
                    std::getline(v, index, '/'); // [/]区切りでインデックスを読む
                    if (index.empty())
                    {
                        // ダブルスラッシュの場合
                        index = "0";
                    }
                    elementIndices[element] = std::stoi(index);
                }
                // 要素へのIndexから、実際の要素の値を取得して、頂点を構築する
                Vector4 position = positions[elementIndices[0] - 1];

                Vector2 texcoord = {};
                if (texcoords.empty())
                {
                    int32_t func = faceVertex - 1;
                    texcoord = (Vector2(static_cast<float>(func * func), static_cast<float>(faceVertex) * 0.5f));
                }
                else texcoord = texcoords[elementIndices[1] - 1];

                Vector3 normal = normals[elementIndices[2] - 1];
                triangle[faceVertex] = { position, texcoord, normal };
            }
            modelData.vertices.push_back(triangle[2]);
            modelData.vertices.push_back(triangle[1]);
            modelData.vertices.push_back(triangle[0]);
        }
        else if (identifier == "mtllib" && texturePath.empty())
        {
            // materialTemplateLibraryファイルの名前を取得
            std::string materialFilename;
            s >> materialFilename;
            // 基本的にobjファイルと同一階層にmtlを配置するためディレクトリ名とファイル名を渡す
            modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
        }
        else if (identifier == "mtllib" && !texturePath.empty())
        {
            std::string materialFilename;
            s >> materialFilename;
            LoadMaterialTemplateFile(directoryPath, materialFilename, texturePath);
        }
    }
    // 4 Return ModelData
    return modelData;
}

MaterialData Helper::Model::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename, const std::string& texturePath)
{
    // 1 Decleare variable
    MaterialData materialData;
    std::string line;
    // 2 Open file
    std::ifstream file(directoryPath + "/" + filename);
    assert(file.is_open());
    // 3 Read file and construct MaterialData
    while (std::getline(file, line))
    {
        std::string identifier;
        std::istringstream s(line);
        s >> identifier;

        // identifierに応じた処理
        if (identifier == "map_Kd" && texturePath.empty())
        {
            std::string textureFilename;
            s >> textureFilename;
            // 連結してファイルパスに
            materialData.textureFilePath = directoryPath + "/" + textureFilename;
        }
        else if (identifier == "map_Kd" && !texturePath.empty())
        {
            materialData.textureFilePath = texturePath;
        }
        else if (identifier == "Kd")
        {
            // 拡散反射率を取得
            for (int32_t numRGB = 0; numRGB < 3; numRGB++)
            {
                std::string diffuseDefinition;
                s >> diffuseDefinition;
                *(&materialData.diffuse.x + numRGB) = std::stof(diffuseDefinition, nullptr);
            }
            materialData.diffuse.w = 1.0f;
        }
    }
    // 4 Return MaterialData
    return materialData;
}

void Helper::Model::DispatchModel(IModel* pModel)
{
    auto ptr = dynamic_cast<GltfModel*>(pModel);
    if (!ptr) return;

    ptr->DispatchSkinning();
}

std::unique_ptr<ModelStorage> Helper::Model::CreateStorage()
{
    return std::make_unique<ModelStorage>();
}

std::unique_ptr<ModelManager> Helper::Model::CreateManager(IModelLoader* pLoader, ModelStorage* pStorage)
{
    auto pModelManager = std::make_unique<ModelManager>();
    pModelManager->Initialize(pLoader, pStorage);
    return pModelManager;
}
