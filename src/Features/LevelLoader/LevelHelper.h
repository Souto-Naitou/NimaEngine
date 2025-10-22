#pragma once
#include <string>

#include <Features/LevelLoader/SceneObjects.h>
#include <Features/Model/ModelManager.h>

/// <summary>
/// レベルヘルパー関数群
/// </summary>
namespace Helper::Level
{
    SceneObjects LoadScene(const std::string& _path, ModelManager* _pModelManager);
    void Unload(const std::string& _path);
}