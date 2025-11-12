#include "Bar2d.h"
#include <Core/DirectX12/TextureManager.h>
#include <imgui.h>

#define TO_VECTOR2(class) Vector2{(class).x, (class).y}
#define TO_VECTOR4(class) Vector4{(class).x, (class).y, (class).z, (class).w}

const RGBA Bar2d::COLOR_BAR_NORMAL = RGBA(0x2762e8ff);
const RGBA Bar2d::COLOR_BAR_BG = RGBA(0xabababff);
const RGBA Bar2d::COLOR_BAR_LOW = RGBA(0x2762e8ff);

const Vector2 Bar2d::SPACING_HEAD_TO_DECO = { 0.0f, 10.0f };

Bar2d::~Bar2d()
{
}

void Bar2d::Initialize(const Bar2dInitParams& params)
{
    initParams_ = params;

    pDebugEntry_ = std::make_unique<DebugEntry<Bar2d>>("Bar2d", "unnamed", this, false);

    isDisplay_name_ = !params.nameTexturePath.empty();

    // テクスチャの読み込み
    auto* tm = TextureManager::GetInstance();
    tm->LoadTexture(PATH_BAR);
    tm->LoadTexture(PATH_DECORATION);
    if (isDisplay_name_) tm->LoadTexture(params.nameTexturePath);

    // スプライトの初期化
    bar_ = std::make_unique<Sprite>();
    bar_->Initialize(PATH_BAR);
    bar_->SetColor(COLOR_BAR_NORMAL.to_Vector4());
    
    if (isDisplay_name_)
    {
        name_ = std::make_unique<Sprite>();
        name_->Initialize(params.nameTexturePath);
    }

    background_ = std::make_unique<Sprite>();
    background_->Initialize(PATH_BAR);
    background_->SetColor(COLOR_BAR_BG.to_Vector4());
}

void Bar2d::Finalize()
{
}

void Bar2d::Update()
{
    this->UpdateTransform();

    if (maxValue_ != 0.0f)
    {
        float t = (currentValue_ - 0.1f) / (maxValue_ + 0.1f);
        if (t > 1.0f) t = 1.0f;
        if (t < 0.0f) t = 0.0f;
    }
    
    this->UpdateColor();

    if (bar_) bar_->Update();
    if (background_) background_->Update();
    if (name_) name_->Update();
}

void Bar2d::Draw1F()
{
    if (background_) background_->Draw1F();
    if (bar_) bar_->Draw1F();
    if (name_) name_->Draw1F();
}

void Bar2d::ImGui()
{
    #ifdef _DEBUG

    ImGui::Text("Position");
    ImGui::DragFloat2("Position", &position_.x, 0.1f);
    ImGui::Text("Max Value");
    ImGui::DragFloat("Max Value", &maxValue_, 0.1f, FLT_MIN);
    ImGui::Text("Current Value");
    ImGui::DragFloat("Current Value", &currentValue_, 0.1f, FLT_MIN, maxValue_);

    #endif
}

void Bar2d::SetOpacity(float alpha)
{
    if (bar_)
    {
        Vector4 color = TO_VECTOR4(bar_->GetColor());
        color.w = alpha;
        bar_->SetColor(TO_VECTOR4(color));
    }

    if (background_)
    {
        Vector4 color = TO_VECTOR4(background_->GetColor());
        color.w = alpha;
        background_->SetColor(TO_VECTOR4(color));
    }

    if (name_)
    {
        Vector4 color = TO_VECTOR4(name_->GetColor());
        color.w = alpha;
        name_->SetColor(TO_VECTOR4(color));
    }
}

void Bar2d::UpdateTransform()
{
    Vector2 leftTop = position_ - mul(anchor_, initParams_.barSize);
    Vector2 cPos = leftTop;

    if (name_) 
    {
        name_->SetPosition(cPos);
        cPos.y += name_->GetSize().y;
    }

    bar_->SetPosition(cPos);

    float ratio = currentValue_ / maxValue_;
    bar_->SetSize({ initParams_.barSize.x * ratio, initParams_.barSize.y });

    background_->SetPosition(cPos);
    background_->SetSize(initParams_.barSize);

    cPos.x += initParams_.barSize.x;
}

void Bar2d::UpdateColor()
{
    if (initParams_.enableSmoothingColor)
    {
        Vector4 color = {};
        color.Lerp(COLOR_BAR_LOW.to_Vector4(), COLOR_BAR_NORMAL.to_Vector4(), currentValue_ / maxValue_);
        bar_->SetColor(TO_VECTOR4(color));
    }
    else
    {
        if (currentValue_ < maxValue_ * BORDER_DANGER)
        {
            bar_->SetColor(COLOR_BAR_LOW.to_Vector4());
        }
        else
        {
            bar_->SetColor(COLOR_BAR_NORMAL.to_Vector4());
        }
    }
}

Vector2 Bar2d::mul(const Vector2& lv, const Vector2& rv)
{
    return Vector2(lv.x * rv.x, lv.y * rv.y);
}
