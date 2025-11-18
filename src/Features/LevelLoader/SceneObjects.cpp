#include "./SceneObjects.h"
#include <Features/Model/ObjModel.h>
#include <utility>
#include <Features/GameEye/FreeLook/FreeLookEye.h>
#include <drawable/object3d/Object3dSystem.h>
#include <imgui.h>
#include <DebugTools/DebugManager/DebugManager.h>

SceneObjects::SceneObjects()
{
    RegisterDebugWindowC("SceneObjects", name_, SceneObjects::ImGui, false);
}

SceneObjects::~SceneObjects()
{
    UnregisterDebugWindowC("SceneObjects", name_);
}

void SceneObjects::Initialize()
{
}

void SceneObjects::Finalize()
{
    for (auto& obj : objects_)
    {
        obj->Finalize();
    }
    objects_.clear();
}

void SceneObjects::Update()
{
    for (auto& model : models_)
    {
        if (model)
        {
            model->Update();
        }
    }

    for (auto& obj : objects_)
    {
        if (obj)
        {
            obj->Update();
        }
    }

    gameeye_->Update();
}

void SceneObjects::Draw()
{
    for (const auto& obj : objects_)
    {
        if (obj)
        {
            obj->Draw1F();
        }
    }
}

std::string SceneObjects::GetName() const
{
    return levelData_.name.empty() ? "Unnamed Scene" : levelData_.name;
}

#ifdef _DEBUG
void ImGuiTextTransform(const EulerTransform& _tf)
{
    #ifdef _DEBUG
    const auto& [scale, rotate, translate] = _tf;
    ImGui::Text("Scale: (%.2f, %.2f, %.2f)", scale.x, scale.y, scale.z);
    ImGui::Text("Rotate: (%.2f, %.2f, %.2f)", rotate.x, rotate.y, rotate.z);
    ImGui::Text("Translate: (%.2f, %.2f, %.2f)", translate.x, translate.y, translate.z);
    #else 
    _tf;
    #endif // _DEBUG
}
#endif // _DEBUG

void SceneObjects::ImGui()
{
    #ifdef _DEBUG

    if (objects_.empty()) return;

    ImGui::Spacing();
    ImGui::SeparatorText("Scene Objects");
    for (const auto& obj : levelData_.objects)
    {
        const auto& [type, name, transform, filename, collider, uvScale] = obj;

        std::string lavel = std::format("[{}] {}", type, name);
        if (ImGui::TreeNode(lavel.c_str()))
        {
            if (!filename.empty()) ImGui::Text("filename : %s", filename.c_str());
            if (ImGui::TreeNode("Transform"))
            {
                ImGuiTextTransform(transform);
                ImGui::TreePop();
            }
            ImGui::Text("UV Scale : (%.2f, %.2f)", uvScale.x, uvScale.y);
            ImGui::TreePop();
        }
    }
  
    #endif // _DEBUG
}

void SceneObjects::SetLevelData(const BlenderLevel::LevelData& _levelData)
{
    levelData_ = _levelData;
}

void SceneObjects::SetDirectionalLight(DirectionalLight* _light)
{
    for (auto& obj : objects_)
    {
        if (obj)
        {
            auto& option = obj->GetOption();
            option.lightingData->enableLighting = true;
            obj->SetDirectionalLight(_light);
        }
    }
}

void SceneObjects::Build(ModelManager* _modelManager)
{
    for (const auto& object : levelData_.objects)
    {
        if (object.type == "MESH")
        {
            // Create a model from the object data
            IModel* model = nullptr;
            try { model = _modelManager->Load(object.filename); }
            catch (const std::runtime_error& e)
            {
                Logger::GetInstance()->LogError(__FILE__, __FUNCTION__, e.what());
            }

            name_ = object.name.empty() ? "Unnamed Scene" : object.name;

            std::shared_ptr<ObjModel> objModel = std::make_shared<ObjModel>();
            objModel->Clone(model);
            models_.emplace_back(std::move(objModel));

            // Create an Object3d instance
            std::shared_ptr<Object3d> object3d = std::make_shared<Object3d>();
            object3d->Initialize();
            object3d->SetModel(models_.back().get());
            object3d->SetName(object.name);
            auto& option = object3d->GetOption();
            option.transform = object.transform;
            option.tilingData->tilingMultiply = object.uvScale;
            option.lightingData->enableLighting = false;
            objects_.emplace_back(std::move(object3d));
        }
        else if (object.type == "CAMERA")
        {
            gameeye_ = std::make_shared<FreeLookEye>();
            gameeye_->SetTransform(object.transform);
            gameeye_->SetName(object.name);
        }
    }

    // GameEyeを渡す
    Object3dSystem::GetInstance()->SetGlobalEye(gameeye_.get());
}