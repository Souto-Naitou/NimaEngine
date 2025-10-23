#pragma once

/// <summary>
/// ライティング管理クラス
/// </summary>
class LightingManager
{
public:
    LightingManager(LightingManager&) = delete;
    LightingManager(LightingManager&&) = delete;
    LightingManager& operator=(LightingManager&) = delete;
    LightingManager& operator=(LightingManager&&) = delete;

    static LightingManager* GetInstance()
    {
        static LightingManager instance;
        return &instance;
    };


public:
    void RegisterPointLight() {};


public:
    void PresentDraw();

private:
    LightingManager() = default;
    ~LightingManager() = default;

};