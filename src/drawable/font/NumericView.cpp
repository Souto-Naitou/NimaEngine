#include "NumericView.h"
#include <cmath>
#include <Core/DirectX12/TextureManager.h>

void NumericView::Initialize(std::span<D3D12_GPU_DESCRIPTOR_HANDLE> textureHandles)
{
    assert(textureHandles.size() == kRadix_);

    pDebugEntry_ = std::make_unique<DebugEntry<NumericView>>("Drawable", "NumericView", this);

    for (uint32_t i = 0; i < kRadix_; ++i)
    {
        numberTextureHandles_[i] = textureHandles[i];
    }
    // 最低1桁分のスプライトは用意しておく
    this->AddSprite();
}

void NumericView::Update()
{
    /// スプライトの数を確認
    uint32_t number = currentNumber_;
    /// 桁数を計算
    uint32_t digitCount = this->GetDigitCount();

    /// 桁が多くなり、スプライトが足りない場合は追加する
    if (numberSprites_.size() < digitCount)
    {
        uint32_t countToAdd = digitCount - static_cast<uint32_t>(numberSprites_.size());
        for (uint32_t i = 0; i < countToAdd; ++i)
        {
            this->AddSprite();
        }
    }

    /// 各スプライトに数字を割り当てる
    glyphInfos_.clear();
    for (uint32_t i = 0; i < digitCount; ++i)
    {
        uint32_t digit = number % 10;
        D3D12_GPU_DESCRIPTOR_HANDLE handle = numberTextureHandles_[digit];
        numberSprites_[digitCount - i - 1]->SetTextureHandle(handle);
        /// サイズを設定
        const auto& metadata = TextureManager::GetInstance()->GetMetaData(handle);
        const float aspect = static_cast<float>(metadata.width) / static_cast<float>(metadata.height);
        Vector2 size = { fontSizeY_ * aspect, static_cast<float>(fontSizeY_) };
        GlyphInfo glyphInfo;
        glyphInfo.size = size;
        glyphInfos_.emplace_back(glyphInfo);
        number /= 10;
    }

    /// レイアウト計算
    auto layoutResults = fontLayout_.Compute(glyphInfos_);
    for (uint32_t i = 0; i < digitCount; ++i)
    {
        numberSprites_[i]->SetPosition(layoutResults[i].leftTop);
        numberSprites_[i]->SetSize(glyphInfos_[i].size);
        numberSprites_[i]->Update();
    }
}

void NumericView::Draw1F()
{
    uint32_t digitCount = this->GetDigitCount();
    for (uint32_t i = 0; i < digitCount; ++i)
    {
        auto& sprite = numberSprites_[i];
        sprite->Draw1F();
    }
}

void NumericView::ImGui()
{
    #ifdef _DEBUG

    ImGui::DragFloat2("FontLayout LeftTop", &fontLayout_.GetProperties().leftTop.x, 1.0f);
    ImGui::DragFloat("FontLayout LetterSpacing", &fontLayout_.GetProperties().letterSpacing, 0.1f);
    ImGui::DragFloat("FontSize", &fontSizeY_, 0.01f, 1.0f, 1000.0f);
    ImGui::DragInt("Current Number", reinterpret_cast<int*>(&currentNumber_), 1.0f, 0, 1000000);

    #endif // _DEBUG
}

void NumericView::SetFontSize(float sizeY)
{
    fontSizeY_ = sizeY;
}

void NumericView::AddSprite()
{
    auto sprite = std::make_unique<Sprite>();
    sprite->Initialize(numberTextureHandles_[0]);
    numberSprites_.emplace_back(std::move(sprite));
}

uint32_t NumericView::GetDigitCount() const
{
    if (currentNumber_ == 0) return 1;
    else return static_cast<uint32_t>(std::log10(currentNumber_)) + 1;
}
