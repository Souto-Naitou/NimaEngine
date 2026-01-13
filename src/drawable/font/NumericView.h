#pragma once
#include <d3d12.h>

#include <array>
#include <span>
#include <vector>
#include <drawable/sprite/Sprite.h>
#include <Features/font/FontLayout.h>
#include <memory>

class NumericView
{
public:
    void Initialize(std::span<D3D12_GPU_DESCRIPTOR_HANDLE> textureHandles);
    void Update();
    void Draw1F();
    void ImGui();

    void SetFontSize(float sizeY);
    void SetNumber(uint32_t number) { currentNumber_ = number; }
    void SetColor(const Vector4& color);
    FontLayout::Properties& GetFontLayoutProperties() { return fontLayout_.GetProperties(); }
    Vector4 GetColor() const { return color_; }

private:
    void AddSprite();
    uint32_t GetDigitCount() const;
    static constexpr uint32_t kRadix_ = 10u;

    std::unique_ptr<DebugEntry<NumericView>> pDebugEntry_ = nullptr;
    std::array<D3D12_GPU_DESCRIPTOR_HANDLE, kRadix_> numberTextureHandles_;

    FontLayout fontLayout_;
    std::vector<std::unique_ptr<Sprite>> numberSprites_;
    std::vector<GlyphInfo> glyphInfos_;

    float fontSizeY_ = 32.0f;

    Vector4 color_ = {1.0f, 1.0f, 1.0f, 1.0f};

    uint32_t currentNumber_ = 0;
};