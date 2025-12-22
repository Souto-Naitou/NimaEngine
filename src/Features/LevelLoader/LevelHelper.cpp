#include "./LevelHelper.h"

#include <Utility/JSONIO/jsonio.h>
#include <numbers>

SceneObjects Helper::Level::LoadScene(const std::string& path, ModelManager* pModelManager)
{
    JSONIO* jsonio = JSONIO::GetInstance();
    const auto& json = jsonio->Load(path);
    BlenderLevel::LevelData levelData;
    levelData = json;
    
    for (auto& object : levelData.objects)
    {
        EulerTransform temp;
        temp = object.transform;
        
        // スケールと平行移動：軸の入れ替え（Y ↔ Z）および X軸反転
        object.transform.scale.y = temp.scale.z;
        object.transform.scale.z = temp.scale.y;
        object.transform.translate.x = -temp.translate.x;
        object.transform.translate.y = temp.translate.z;
        object.transform.translate.z = temp.translate.y;
        
        // X軸周りに90度回転を追加して向きを調節
        object.transform.rotate.x = temp.rotate.x - std::numbers::pi_v<float> / 2.0f;
        object.transform.rotate.y = -temp.rotate.z;
        object.transform.rotate.z = temp.rotate.y;

        if (object.type == "MESH")
        {
            object.transform.rotate.y -= std::numbers::pi_v<float>; // 180度回転
        }
    }

    SceneObjects sceneObjects;
    sceneObjects.Initialize();
    sceneObjects.SetLevelData(levelData);
    sceneObjects.Build(pModelManager);
    return std::move(sceneObjects);
}

void Helper::Level::Unload(const std::string& path)
{
    JSONIO* jsonio = JSONIO::GetInstance();
    jsonio->Unload(path);
}