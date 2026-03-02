#include "Posterize.h"
#include <imgui.h>
#include <Core/DirectX12/Helper/DX12Helper.h>



void Posterize::Initialize(const PostEffectInitParams& params)
{
    this->SetName("Posterize");
    this->SetPSOId("Posterize");
    this->SetRootSignatureId("Posterize");
    this->SetPixelShaderPath(L"EngineResources/Shaders/Posterize.PS.hlsl");

    this->PostEffectBase::Initialize(params);
}

void Posterize::DebugOverlay()
{
    #ifdef _DEBUG
    ImGui::DragFloat("Level", &pOption_->level, 0.01f, 1.0f, 256.0f);
    #endif // _DEBUG
}

void Posterize::CreateCBuffer(ID3D12Device* pDevice)
{
    pOptionResource_ = DX12Helper::CreateBufferResource(pDevice, sizeof(PosterizeOption));
    pOptionResource_->Map(0, nullptr, reinterpret_cast<void**>(&pOption_));

    pOption_->level = 8.0f; // 定数バッファの初期値を設定
}

void Posterize::SetCBuffer(ID3D12GraphicsCommandList* cl)
{
    cl->SetGraphicsRootConstantBufferView(2, pOptionResource_->GetGPUVirtualAddress());
}

void Posterize::RegisterAdditionalRootParameter(RootParameters& rootParams)
{
    rootParams.SetParameter(2, "b1", D3D12_SHADER_VISIBILITY_PIXEL);
}
