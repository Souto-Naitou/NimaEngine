#pragma once

#include <Core/Localization/LanguageData.h>
#include <Core/DirectX12/DirectX12.h>
#include <Features/TimeMeasurer/TimeMeasurer.h>

#include <functional>
#include <list>
#include <string>
#include <array>
#include <optional>
#include <Features/Input/Input.h>

#ifdef _DEBUG
#define RegisterDebugWindowC(category, name, func, windowMode)      DebugManager::GetInstance()->SetComponent(category, name, std::bind(&func, this), windowMode)
#define RegisterDebugWindowS(name, func, windowMode)                DebugManager::GetInstance()->SetComponent(name, std::bind(&func, this), windowMode)
#define UnregisterDebugWindowC(category, name)                      DebugManager::GetInstance()->DeleteComponent(category, name)
#define UnregisterDebugWindowS(name)                                DebugManager::GetInstance()->DeleteComponent(name)
#else
#define RegisterDebugWindowC(category, name, func, windowMode)      
#define RegisterDebugWindowS(name, func, windowMode)                
#define UnregisterDebugWindowC(category, name)                      
#define UnregisterDebugWindowS(name)                                
#endif // _DEBUG

/// <summary>
/// デバッグマネージャー
/// </summary>
class DebugManager : public EngineFeature
{
    struct ComponentData
    {
        std::optional<std::string>  categoryId  = {};
        const std::string*          id_ptr      = nullptr;
        std::string                 id_cpy      = {};
        std::function<void(void)>   function    = {};
        bool                        isEnabled   = false;
        bool                        isWindow    = false;
    };

public:
    static DebugManager* GetInstance();

    DebugManager(const DebugManager&) = delete;
    DebugManager& operator=(const DebugManager&) = delete;
    DebugManager(DebugManager&&) = delete;
    DebugManager& operator=(const DebugManager&&) = delete;

    /// <summary>
    /// デバッグ用コンポーネントの登録
    /// </summary>
    /// <param name="_category">カテゴリ</param>
    /// <param name="_name">オブジェクト名</param>
    /// <param name="_component">関数ポインタ。std::bindがおすすめ</param>
    void    SetComponent(const std::string& _category, const std::string& _name, const std::function<void(void)>& _component, bool isWindowMode = false);
    void    SetComponent(const std::string& _category, const std::string&& _name, const std::function<void(void)>& _component, bool isWindowMode = false);
    void    SetComponent(const std::string& _name, const std::function<void(void)>& _component, bool isWindowMode = false);
    void    SetComponent(const std::string&& _name, const std::function<void(void)>& _component, bool isWindowMode = false);

    void    DeleteComponent(const std::string& _name);
    void    DeleteComponent(const std::string& _category, const std::string& _name);
    void    DeleteComponent(const std::string& _category, const std::string&& _name);

    void    Update();
    void    DrawUI();
    void    ChangeFont();
    void    SetDisplay(bool _isEnable) { isDisplay_ = _isEnable; }
    bool    IsDisplay() const { return isDisplay_; }

    double  GetFPS() const { return fps_; }
    void    PushLog(const std::string& _log);

private:
    DebugManager();
    ~DebugManager();

    // Input
    Input*                  pInput_ = nullptr;

    // Localization
    Localization::_Common       lang_common_ = {};
    Localization::_DebugManager lang_dm_ = {};

    // Component data structure
    std::list<ComponentData>    componentList_;

    // Timing utilities for frame measurement
    TimeMeasurer            timer_ = {};
    TimeMeasurer            frameTimer_ = {};

    // FPS calculation data
    double                  elapsedFrameCount_ = 0.0;
    double                  fps_ = 0.0;
    std::array<float, 120>  fpsList_ = {};
    unsigned int            frameCount_ = 0u;
    double                  frameTime_ = 0.0;

    // Log storage
    std::string             textLog_ = {};

    // flags
    bool                    isDisplay_ = true;
    bool                    enableAutoScroll_ = true;
    bool                    isExistSettingFile_ = false;
    

private:
    void MeasureFPS();
    void MeasureFrameTime();


private: /// Windows
    void Window_ObjectList();
    void Window_DebugInfo();
    void Window_Inspector();
    void ShowDockSpace();
    void OverlayFPS() const;
    void DebugInfoBar() const;
};