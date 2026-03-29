#pragma once

#include <Core/Localization/LanguageData.h>
#include <Core/DirectX12/DirectX12.h>
#include <Features/TimeMeasurer/TimeMeasurer.h>

#include <functional>
#include <list>
#include <string>
#include <array>
#include <optional>
#include <unordered_map>
#include <utility>
#include <Features/Viewport/Viewport.h>
#include <DebugTools/Logger/Logger.h>
#include <DebugTools/EventTimer/EventTimer.h>

class Input;

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

    /// ビューポートのインスタンスを登録
    void    SetViewportWindow(Viewport* pViewport);
    /// Logger のインスタンスを登録
    void    SetLoggerWindow(Logger* pLogger);
    /// イベントタイマーのインスタンスを登録
    void    SetEventTimerWindow(EventTimer* pEventTimer);

    void    DeleteComponent(const std::string& _name);
    void    DeleteComponent(const std::string& _category, const std::string& _name);
    void    DeleteComponent(const std::string& _category, const std::string&& _name);

    void    Update();
    void    DrawUI();
    void    ChangeFont();
    void    SetDisplay(bool _isEnable)  { isDisplay_ = _isEnable; }
    bool    IsDisplay() const           { return isDisplay_; }
    double  GetFPS() const              { return fps_; }

private:
    /// <summary>
    /// ImGuiウィンドウ名列挙型
    /// </summary>
    enum class WindowType
    {
        ObjectList,
        DebugInfo,
        DebugInfoBar,
        Inspector,
        OverlayFPS,
        Logger,
        Viewport,
        EventTimer,
        NiGuiDebug,
    };

    DebugManager();
    ~DebugManager();

    // Input
    Input*                      pInput_ = nullptr;

    // Localization
    Localization::Common        lang_common_ = {};
    Localization::DebugManager  lang_dm_ = {};

    // Component data structure
    std::list<ComponentData>    componentList_;
    Viewport*                   pViewport_      = nullptr;
    Logger*                     pLogger_        = nullptr;
    EventTimer*                 pEventTimer_    = nullptr;

    // Timing utilities for frame measurement
    TimeMeasurer                timer_          = {};
    TimeMeasurer                frameTimer_     = {};

    // FPS calculation data
    double                      elapsedFrameCount_ = 0.0;
    double                      fps_            = 0.0;
    std::array<float, 120>      fpsList_        = {};
    unsigned int                frameCount_     = 0u;
    double                      frameTime_      = 0.0;

    // Log storage
    std::string                 textLog_        = {};

    using WindowFunc = std::function<void(DebugManager&)>;
    using WindowFuncPair = std::pair<bool, WindowFunc>;
    std::unordered_map<WindowType, WindowFuncPair> windowFuncs_;

    // flags
    bool                    isDisplay_ = true;
    bool                    enableAutoScroll_ = true;
    bool                    isExistSettingFile_ = false;

private:
    void MeasureFPS();
    void MeasureFrameTime();
    void SwitchEnableWindow();

private: /// Windows
    void Window_ObjectList();
    void Window_DebugInfo();
    void Window_Inspector();
    void ShowDockSpace();
    void OverlayFPS() const;
    void Window_DebugInfoBar() const;
};