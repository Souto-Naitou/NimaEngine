#include "ChromaticAberration.h"
#include <Core/DirectX12/Helper/DX12Helper.h>
#include <imgui.h>
#include <cmath>



void ChromaticAberration::Initialize(const PostEffectInitParams& desc)
{
    this->SetName("Chromatic Aberration");
    this->SetPSOId("ChromaticAberration");
    this->SetRootSignatureId("ChromaticAberration");
    this->SetPixelShaderPath(L"EngineResources/Shaders/ChromaticAberration.PS.hlsl");

    this->PostEffectBase::Initialize(desc);
}

void ChromaticAberration::Setting()
{
    if (pOption_->direction.LengthWithoutRoot() < 0.001f)
    {
        pOption_->direction = { 1.0f, 0.0f };
    }

    this->PostEffectBase::Setting();
}

void ChromaticAberration::DebugOverlay()
{
    #ifdef _DEBUG
    if (ImGui::SliderAngle("Direction", &angleRadians_, 0.0f, 359.9f))
    {
        pOption_->direction.x = std::cos(angleRadians_);
        pOption_->direction.y = std::sin(angleRadians_);
    }
    ImGui::DragFloat("Strength", &pOption_->strength, 0.001f, 0.0f, 1.0f);
    #endif // _DEBUG
}

void ChromaticAberration::CreateCBuffer(ID3D12Device* pDevice)
{
    pOptionResource_ = DX12Helper::CreateBufferResource(pDevice, sizeof(ChromaticAberrationOption));
    pOptionResource_->Map(0, nullptr, reinterpret_cast<void**>(&pOption_));

    // 定数バッファの初期値を設定
}

void ChromaticAberration::SetCBuffer(ID3D12GraphicsCommandList* pCommandList)
{
    pCommandList->SetGraphicsRootConstantBufferView(2, pOptionResource_->GetGPUVirtualAddress());
}

void ChromaticAberration::RegisterAdditionalRootParameter(RootParameters& rootParams)
{
    rootParams.SetParameter(2, "b1", D3D12_SHADER_VISIBILITY_PIXEL);
}
