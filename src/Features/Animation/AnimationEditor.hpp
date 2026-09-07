#pragma once
#include <memory>
#include <DebugTools/DebugEntry/DebugEntry.h>
#include <Features/Animation/AnimationTimeline.hpp>
#include <string>
#include <imgui.h>
#include <utility>
#include <Features/Animation/AnimationSerializer.hpp>
#include <DebugTools/ImGuiTemplates/ImGuiTemplates.h>

class AnimationEditor
{
public:
    /// タイムラインの登録を管理するクラス
    class Registration
    {
    public:
        Registration() = default;

        // ctor
        Registration(AnimationEditor* e, std::string key) : pEditor_(e), key_(key) {}

        // dtor
        ~Registration() { Reset(); }

        // move ctor
        Registration(Registration&& origin) noexcept { Swap(origin); }

        // move assignment
        Registration& operator=(Registration&& origin) noexcept
        {
            if (this != &origin)
            {
                Reset();
                Swap(origin);
            }
            return *this;
        }

        // copy prohibited
        Registration(const Registration&) = delete;
        Registration& operator=(const Registration&) = delete;

        // Reset the registration
        void Reset()
        {
            if (pEditor_) pEditor_->drawers_.erase(key_);
            pEditor_ = nullptr;
            key_.clear();
        }

    private:
        // Swap the contents of two Registration objects
        void Swap(Registration& origin) noexcept
        {
            std::swap(pEditor_, origin.pEditor_);
            std::swap(key_, origin.key_);
        }

        AnimationEditor* pEditor_ = nullptr;
        std::string key_ = {};
    };

    template <typename ValueType>
    [[nodiscard]] Registration Register(const std::string& name, AnimationTimeline<ValueType>& timeline)
    {
        drawers_[name] = [this, name, &timeline] { DrawTimeline(name, timeline); };

        auto& fileWidget = fileWidgets_[name];

        fileWidget.SetOnSave([&timeline](const std::string& path)
        {
            AnimationSerializer::Save<ValueType>(path, timeline);
        });

        fileWidget.SetOnLoad([&timeline](const std::string& path)
        {
            timeline = AnimationSerializer::Load<ValueType>(path);
        });

        return { this, name };
    }

    AnimationEditor()
    {
        #ifdef _DEBUG
        pDebugEntry_ = std::make_unique<DebugEntry<AnimationEditor>>("AnimationEditor", this, false);
        #endif // _DEBUG
    }

    ~AnimationEditor() = default;

    // インスタンスを取得
    static AnimationEditor* GetInstance()
    {
        static auto instance = new AnimationEditor();
        return instance;
    }

    void ImGui()
    {
        for (auto& [name, drawer] : drawers_)
        {
            drawer();
        }
    }

private:
    template <typename ValueType>
    void DrawTimeline(const std::string& name, AnimationTimeline<ValueType>& timeline);

    std::unordered_map<std::string, std::function<void()>> drawers_;
    std::unordered_map<std::string, ImGuiTemplate::FileWidget> fileWidgets_;
    std::unique_ptr<DebugEntry<AnimationEditor>> pDebugEntry_ = nullptr;

    friend class Registration;
};

template <typename ValueType>
void AnimationEditor::DrawTimeline(const std::string& name, AnimationTimeline<ValueType>& timeline)
{
    #ifdef _DEBUG

    const float timelineDuration = timeline.GetTimelineDurationSec();
    const auto targetValueLast = timeline.GetTargetValueLast();
    auto& tweens = timeline.GetTweens();

    bool isFileExist = false;
    if (ImGui::TreeNode(name.c_str()))
    {
        // 読み込み、保存、参照のUIを表示
        fileWidgets_[name].ImGui(name);

        ImGui::Indent(15.0f);

        if (ImGui::Button("Play")) timeline.Start();
        ImGui::SameLine();
        if (ImGui::Button("Add")) timeline.AddTween(timelineDuration, 1.0f, targetValueLast, targetValueLast);

        uint32_t index = 0;
        for (auto it = tweens.begin(); it != tweens.end();)
        {
            ImGui::PushID(index);

            bool isErase = false;
            ImGui::Separator();
            if (ImGui::Button("Delete"))
            {
                it = tweens.erase(it);
                isErase = true;
            }
            else
            {
                ImGui::SameLine();
                it->ImGui("Tween " + std::to_string(index));
            }
            ++index;
            ImGui::Separator();

            if (!isErase) ++it;

            ImGui::PopID();
        }

        ImGui::Unindent(15.0f);
        ImGui::TreePop();
    }

    #endif // _DEBUG
}
