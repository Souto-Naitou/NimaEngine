#pragma once

#include <list>
#include <memory>
#include <d3d12.h>
#include <string>
#include <Features/Object3d/Object3d.h>
#include <Features/LevelLoader/ISceneObjects.h>
#include <Features/LevelLoader/LevelData.h>
#include <Features/Model/ModelManager.h>
#include <Features/Model/IModel.h>
#include <Features/GameEye/GameEye.h>
#include <Common/structs.h>

/// <summary>
/// JSONから読み込んだシーンオブジェクト群を保持するクラス
/// </summary>
class SceneObjects : public ISceneObjects
{
public:
    SceneObjects();
    ~SceneObjects();

    void Initialize() override;
    void Finalize() override;
    void Update() override;
    void Draw() override;

    std::string GetName() const override;
    void ImGui();

    void SetLevelData(const BlenderLevel::LevelData& _levelData);
    void SetDirectionalLight(DirectionalLight* _light);
    void Build(ModelManager* _modelManager);

private:
    std::list<std::shared_ptr<Object3d>> objects_;
    std::list<std::shared_ptr<IModel>> models_;
    std::shared_ptr<GameEye> gameeye_; // ゲーム目のオブジェクト
    BlenderLevel::LevelData levelData_; // レベルデータ
    std::string name_ = "empty"; // シーン名
};