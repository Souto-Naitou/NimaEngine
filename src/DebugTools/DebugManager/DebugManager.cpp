#include "DebugTools/DebugManager/DebugManager.h"

#include <NiGui.h>

#ifdef _DEBUG
#include <imgui.h>
#include <imgui_impl_dx12.h>
#endif // _DEBUG

#include <unordered_map>

DebugManager* DebugManager::GetInstance()
{
    static DebugManager instance;
    return &instance;
}

DebugManager::DebugManager()
{
    pInput_ = Input::GetInstance();

    windowFuncs_ = {
        { WindowType::ObjectList,       { true, [](DebugManager& m) { m.Window_ObjectList(); } } },
        { WindowType::DebugInfo,        { true, [](DebugManager& m) { m.Window_DebugInfo(); } } },
        { WindowType::DebugInfoBar,     { true, [](DebugManager& m) { m.Window_DebugInfoBar(); } } },
        { WindowType::Inspector,        { true, [](DebugManager& m) { m.Window_Inspector(); } } },
        { WindowType::OverlayFPS,       { false, [](DebugManager& m) { m.OverlayFPS(); } } },
        { WindowType::Logger,           { false, [](DebugManager& m) {m;} } },
        { WindowType::Viewport,         { false, [](DebugManager& m) {m;} } },
        { WindowType::EventTimer,       { false, [](DebugManager& m) {m;} } },
        { WindowType::NiGuiDebug,       { false, [](DebugManager& m) { m; NiGui::DrawDebug(); } } },
    };
}

DebugManager::~DebugManager()
{

}

void DebugManager::OverlayFPS() const
{
    #ifdef _DEBUG
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

    const float PAD = 10.0f;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 work_pos = viewport->WorkPos;
    ImVec2 work_size = viewport->WorkSize;
    ImVec2 window_pos, window_pos_pivot;
    window_pos.x = work_size.x - PAD;
    window_pos.y = PAD;
    ImGui::SetNextWindowViewport(viewport->ID);
    window_flags |= ImGuiWindowFlags_NoMove;

    ImGui::SetNextWindowBgAlpha(0.0f);

    if (ImGui::Begin(lang_dm_.window_overlayFPS.c_str(), nullptr, window_flags))
    {
        ImGui::Text("%.2lfFPS", fps_);
        ImGui::SameLine();
        ImGui::ProgressBar(static_cast<float>(fps_) / 60.0f, ImVec2(0, 0), "");

        auto wndSize = ImGui::GetWindowSize();
        ImGui::SetWindowPos(ImVec2(work_size.x - wndSize.x - PAD, PAD));
    }
    ImGui::End();
    #endif // _DEBUG
}

void DebugManager::MeasureFPS()
{
    if (!timer_.GetIsStart())
    {
        timer_.Start();
    }
    /// フレームレート計算
    if (timer_.GetNow<double>() - elapsedFrameCount_ >= 0.1)
    {
        fps_ = frameCount_ * 1.0 / (timer_.GetNow<double>() - elapsedFrameCount_);
        frameCount_ = 0;
        elapsedFrameCount_ = timer_.GetNow<double>();
    }
    frameCount_++;

    std::rotate(fpsList_.begin(), fpsList_.begin() + 1, fpsList_.end());
    fpsList_.back() = static_cast<float>(fps_);
}

void DebugManager::MeasureFrameTime()
{
    #ifdef _DEBUG

    frameTime_ = frameTimer_.GetNow<double>();
    frameTimer_.Reset();
    frameTimer_.Start();

    #endif // _DEBUG
}

void DebugManager::SwitchEnableWindow()
{
    // デバッグウィンドウをすべて表示
    if (pInput_->TriggerKey(DIK_F1))
    {
        isDisplay_ = true;
        windowFuncs_[WindowType::DebugInfo].first       = true;
        windowFuncs_[WindowType::DebugInfoBar].first    = true;
        windowFuncs_[WindowType::Inspector].first       = true;
        windowFuncs_[WindowType::ObjectList].first      = true;
        windowFuncs_[WindowType::Logger].first          = true;
        windowFuncs_[WindowType::Viewport].first        = true;
        windowFuncs_[WindowType::EventTimer].first      = true;
    }

    // デバッグウィンドウの表示/非表示切り替え
    if (pInput_->TriggerKey(DIK_F3))
    {
        isDisplay_ = !isDisplay_;

        windowFuncs_[WindowType::DebugInfo].first       = isDisplay_;
        windowFuncs_[WindowType::DebugInfoBar].first    = isDisplay_;
        windowFuncs_[WindowType::Inspector].first       = isDisplay_;
        windowFuncs_[WindowType::ObjectList].first      = isDisplay_;
        windowFuncs_[WindowType::Logger].first          = isDisplay_;
        windowFuncs_[WindowType::Viewport].first        = true;
        windowFuncs_[WindowType::EventTimer].first      = isDisplay_;
    }

    // InspectorウィンドウとViewportが独占
    if (pInput_->TriggerKey(DIK_F4))
    {
        windowFuncs_[WindowType::DebugInfo].first       = false;
        windowFuncs_[WindowType::DebugInfoBar].first    = false;
        windowFuncs_[WindowType::Inspector].first       = true;
        windowFuncs_[WindowType::ObjectList].first      = false;
        windowFuncs_[WindowType::Logger].first          = false;
        windowFuncs_[WindowType::Viewport].first        = true;
        windowFuncs_[WindowType::EventTimer].first      = false;
    }
}

void DebugManager::Window_ObjectList()
{
    #ifdef _DEBUG

    ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth;

    std::list<ComponentData*> uncategorizedComponents;
    std::unordered_map<std::string, std::list<ComponentData*>> parentMap;

    for (auto& component : componentList_)
    {
        if (component.categoryId.has_value())
        {
            parentMap[component.categoryId.value()].push_back(&component);
        }
        else
        {
            uncategorizedComponents.push_back(&component);
        }
    }

    ImGui::PushID("WindowComponentList");
    if (ImGui::Begin(lang_dm_.window_componentList.c_str()))
    {
        // カテゴライズされたコンポーネント
        for (auto& [parentName, list] : parentMap)
        {
            if (ImGui::TreeNodeEx(parentName.c_str(), treeNodeFlags))
            {
                for (auto& component : list)
                {
                    std::string id = component->id_cpy;
                    if (component->id_ptr) id = *component->id_ptr;

                    ImGui::Selectable(id.c_str(), &component->isEnabled);
                }
                ImGui::TreePop();
            }
        }

        // 未カテゴリのコンポーネント
        if (!uncategorizedComponents.empty() && ImGui::TreeNodeEx(lang_common_.uncategorized.c_str(), treeNodeFlags))
        {
            for (auto& component : uncategorizedComponents)
            {
                std::string id = component->id_cpy;
                if (component->id_ptr) id = *component->id_ptr;
                ImGui::Selectable(id.c_str(), &component->isEnabled);
            }
            ImGui::TreePop();
        }
    }
    ImGui::End();
    ImGui::PopID();
    #endif // DEBUG
}

void DebugManager::Window_DebugInfo()
{
    #ifdef _DEBUG

    if (ImGui::Begin(lang_dm_.window_Performance.c_str()))
    {
        ImGui::PlotLines("FPS", fpsList_.data(), static_cast<int>(fpsList_.size()), 0, nullptr, 0.0f, 60.0f, ImVec2(0, 80));
    }
    ImGui::End();

    #endif // _DEBUG
}

void DebugManager::Window_Inspector()
{
    #ifdef _DEBUG

    // 登録されていないなら早期リターン
    if (componentList_.size() == 0) return;

    ImGui::Begin(lang_dm_.window_debug.c_str());

    ImGuiTabBarFlags tabFlag = {};
    tabFlag |= ImGuiTabBarFlags_Reorderable;
    tabFlag |= ImGuiTabBarFlags_FittingPolicyResizeDown;
    tabFlag |= ImGuiTabBarFlags_TabListPopupButton;

    ImGui::BeginTabBar("## TABBAR", tabFlag);
    for (auto& component : componentList_)
    {
        bool isCategorized = component.categoryId.has_value();
        std::string id = component.id_ptr ? *component.id_ptr : component.id_cpy;
        std::string parentID = isCategorized ? component.categoryId.value() : lang_common_.uncategorized;

        if (component.isEnabled)
        {
            std::string tabName;
            if (isCategorized == false) tabName = id;
            else tabName = id + " - " + parentID;

            std::string id_str = tabName + "TABITEM";
            ImGui::PushID(id_str.c_str());
            {
                // ウィンドウモード
                if (component.isWindow)
                {
                    if (ImGui::Begin(id.c_str(), &component.isEnabled))
                    {
                        component.function();
                    }
                    ImGui::End();
                }
                // デフォルトモード
                else if (ImGui::BeginTabItem(tabName.c_str(), &component.isEnabled))
                {
                    ImGui::Text((lang_common_.category + ": %s").c_str(), parentID.c_str());
                    ImGui::TextDisabled((lang_common_.name + ": %s").c_str(), id.c_str());
                    ImGui::Separator();
                    component.function();
                    ImGui::EndTabItem();
                }
            }
            ImGui::PopID();
        }
    }
    ImGui::EndTabBar();
    ImGui::End();

    #endif // _DEBUG
}

void DebugManager::SetComponent(const std::string& _name, const std::function<void(void)>& _component, bool isWindowMode)
{
    ComponentData data = {};
    data.id_ptr = &_name;
    data.id_cpy = "Id is registered as a pointer.";
    data.function = _component;
    data.isEnabled = false;
    data.isWindow = isWindowMode;
    componentList_.emplace_back(data);
}

void DebugManager::SetComponent(const std::string&& _name, const std::function<void(void)>& _component, bool isWindowMode)
{
    ComponentData data = {};
    data.id_ptr = nullptr;
    data.id_cpy = _name;
    data.function = _component;
    data.isEnabled = false;
    data.isWindow = isWindowMode;
    componentList_.emplace_back(data);
}

void DebugManager::SetComponent(const std::string& _category, const std::string& _name, const std::function<void(void)>& _component, bool isWindowMode)
{
    ComponentData data = {};
    data.categoryId = _category;
    data.id_ptr = &_name;
    data.id_cpy = "Id is registered as a pointer";
    data.function = _component;
    data.isEnabled = false;
    data.isWindow = isWindowMode;
    componentList_.emplace_back(data);
}

void DebugManager::SetComponent(const std::string& _category, const std::string&& _name, const std::function<void(void)>& _component, bool isWindowMode)
{
    ComponentData data = {};
    data.categoryId = _category;
    data.id_ptr = nullptr;
    data.id_cpy = _name;
    data.function = _component;
    data.isEnabled = false;
    data.isWindow = isWindowMode;
    componentList_.emplace_back(data);
}

void DebugManager::SetViewportWindow(Viewport* pViewport)
{
    pViewport_ = pViewport;
    windowFuncs_[WindowType::Viewport] = { true, [](DebugManager& m) { m.pViewport_->DrawWindow(); } };
}

void DebugManager::SetLoggerWindow(Logger* pLogger)
{
    pLogger_ = pLogger;
    windowFuncs_[WindowType::DebugInfo] = { true, [](DebugManager& m) { m.pLogger_->DrawUI(); } };
}

void DebugManager::SetEventTimerWindow(EventTimer* pEventTimer)
{
    pEventTimer_ = pEventTimer;
    windowFuncs_[WindowType::EventTimer] = { true, [](DebugManager& m) { m.pEventTimer_->ImGui(); } };
}

void DebugManager::DeleteComponent(const std::string& _name)
{
    try
    {
        componentList_.remove_if([_name](const ComponentData& component)
        {
            bool result = false;
            if (!component.categoryId.has_value())
            {
                std::string id = component.id_ptr ? *component.id_ptr : component.id_cpy;
                result = id.compare(_name) == 0;
            }
            return result;
        });
    }
    catch (const std::exception& e)
    {
        // エラー処理: 例外をログに記録するなど
        Logger::GetInstance()->LogError(__FILE__, __FUNCTION__, e.what());
        assert(false && "mismatch categoryID or name");
    }
}

void DebugManager::DeleteComponent(const std::string& _categoryID, const std::string& _name)
{
    try
    {
        const std::string* id_ptr = &_name;
        componentList_.remove_if([_categoryID, id_ptr](const ComponentData& component)
        {
            if (!component.categoryId || !component.id_ptr) return false;
            return component.categoryId->compare(_categoryID) == 0 && component.id_ptr == id_ptr;
        });
    }
    catch (const std::exception& e)
    {
        // エラー処理: 例外をログに記録するなど
        Logger::GetInstance()->LogError(__FILE__, __FUNCTION__, e.what());
        assert(false && "mismatch categoryID or name");
    }
}

void DebugManager::DeleteComponent(const std::string& _category, const std::string&& _name)
{
    try
    {
        componentList_.remove_if([_category, _name](const ComponentData& component)
        {
            if (!component.categoryId) return false;

            std::string parentId = component.categoryId.value();
            std::string childId = component.id_cpy;
            return parentId.compare(_category) == 0 && childId.compare(_name) == 0;
        });
    }
    catch (const std::exception& e)
    {
        // エラー処理: 例外をログに記録するなど
        Logger::GetInstance()->LogError(__FILE__, __FUNCTION__, e.what());
        assert(false && "mismatch categoryID or name");
    }
}

void DebugManager::Update()
{
    this->SwitchEnableWindow();
    this->MeasureFrameTime();
    this->MeasureFPS();
}

void DebugManager::DrawUI()
{
    #ifdef _DEBUG

    ShowDockSpace();

    /// 有効なウィンドウの表示
    for (auto& [type, funcPair] : windowFuncs_)
    {
        if (funcPair.first && funcPair.second)
        {
            funcPair.second(*this);
        }
    }

    #endif // _DEBUG
}

void DebugManager::ChangeFont()
{
    #ifdef _DEBUG

    ImGuiIO& io = ImGui::GetIO();

    ImFontConfig fontcfg;
    fontcfg.MergeMode = 0;
    fontcfg.OversampleH = 1;
    fontcfg.PixelSnapH = 1;
    fontcfg.GlyphOffset = ImVec2(0.0f, 0.0f);

    //io.Fonts->AddFontDefault();

    ImFont* resultFont = io.Fonts->AddFontFromFileTTF(
        "EngineResources/Font/x12y16pxMaruMonica.ttf",
        16,
        &fontcfg,
        io.Fonts->GetGlyphRangesJapanese()
    );

    io.FontDefault = resultFont;

    io.Fonts->Build();
    ImGui_ImplDX12_CreateDeviceObjects();

    #endif // _DEBUG
}

void DebugManager::ShowDockSpace()
{
    #ifdef _DEBUG

    if (!(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable))
    {
        return;
    }

    auto vp = ImGui::GetMainViewport();

    ImGui::DockSpaceOverViewport(ImGui::GetID("Inspector"), vp, ImGuiDockNodeFlags_PassthruCentralNode);

    #endif // _DEBUG
}

void DebugManager::Window_DebugInfoBar() const
{
    #ifdef _DEBUG

    if (ImGui::Begin("DebugInfoBar", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar))
    {
        ImGui::Text("%.2lfFPS", fps_);
        ImGui::SameLine();
        ImGui::ProgressBar(static_cast<float>(fps_) / 60.0f, ImVec2(200, 0), "");
        ImGui::SameLine();
        ImGui::Text("Update: %dms", static_cast<int>(frameTime_ * 1000));
        ImGui::SameLine();
        ImGui::SetCursorPos({ ImGui::GetCursorPosX() + 32.0f , ImGui::GetCursorPosY() });
        ImGui::Text("F3: Toggle debug window");
    }
    ImGui::End();

    #endif // debug
}
