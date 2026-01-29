#pragma once
#include <string>

#include <Features/LevelLoader/SceneObjects.h>
#include <Features/Model/ModelManager.h>

/// <summary>
/// レベルヘルパー関数群
/// </summary>
namespace Helper::Level
{
    std::unique_ptr<SceneObjects> LoadScene(const std::string& path, ModelManager* pModelManager);
    void Unload(const std::string& path);
}