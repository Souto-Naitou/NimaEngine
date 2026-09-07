#pragma once
#include "FlexTypes.h"
#include <DebugTools/DebugEntry/DebugEntry.h>
#include <Vector2.h>
#include <vector>
#include <span>
#include <memory>

class FlexContainer
{
public:
    /// エイリアス
    using Vec2 = Vector2;

    FlexContainer();
    ~FlexContainer() = default;

    void ImGui();

    FlexDirection   direction_      = FlexDirection::Row;
    JustifyContent  justifyContent_ = JustifyContent::FlexStart;
    AlignItems      alignItems_     = AlignItems::FlexStart;
    float           gap_            = 0.0f;

    std::vector<FlexResult> Calculate(const FlexBox& containerBox, std::span<const FlexItem> items) const;

    Vector2 ContainerSize(std::span<FlexItem> items) const;

    void SetName(const std::string& name)
    { 
        #ifdef _DEBUG
        pDebugEntry_->SetName(name);
        #endif // _DEBUG
    }

private:
    std::unique_ptr<DebugEntry<FlexContainer>> pDebugEntry_ = nullptr;

    float MainOf(const Vec2& v) const;
    float CrossOf(const Vec2& v) const;
    Vec2 ToVec2(float main, float cross) const;
    void ApplyJustify(float freeSpace, std::span<const float> itemMainSizes, std::vector<float>& outPosition) const;
    CrossAlignResult ApplyCrossAlign(float itemCross, float containerCross, const FlexItem& item) const;
};