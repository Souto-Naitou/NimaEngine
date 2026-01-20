#include "GltfModelSystem.h"
#include <Core/DirectX12/RootSignature/RootParameters.h>
#include <Core/DirectX12/Helper/DX12Helper.h>

void GltfModelSystem::Initialize()
{
    _CompileShader();
    _CreateRootSignatureCS();
    _CreatePipelineStateCS();
}

void GltfModelSystem::PrepareDispatch()
{
    auto cl = pDx12_->GetCommandList();
    cl->SetComputeRootSignature(rootSignature_.Get());
    cl->SetPipelineState(pso_.Get());
}

void GltfModelSystem::_CompileShader()
{
    IDxcUtils* dxcUtils = pDx12_->GetDxcUtils();
    IDxcCompiler3* dxcCompiler = pDx12_->GetDxcCompiler();
    IDxcIncludeHandler* includeHandler = pDx12_->GetIncludeHandler();

    /// ShaderをCompileする
    computeShaderBlob_ = DX12Helper::CompileShader(kComputeShaderPath, L"cs_6_0", dxcUtils, dxcCompiler, includeHandler);
    assert(computeShaderBlob_ != nullptr);
}

void GltfModelSystem::_CreatePipelineStateCS()
{
    D3D12_COMPUTE_PIPELINE_STATE_DESC computePipelineStateDesc = {};
    computePipelineStateDesc.CS = {
        .pShaderBytecode = computeShaderBlob_->GetBufferPointer(),
        .BytecodeLength = computeShaderBlob_->GetBufferSize()
    };
    computePipelineStateDesc.pRootSignature = rootSignature_.Get();
    HRESULT hr = pDx12_->GetDevice()->CreateComputePipelineState(&computePipelineStateDesc, IID_PPV_ARGS(&pso_));
    if (FAILED(hr))
    {
        Logger::GetInstance()->LogError(
            "GltfModelSystem",
            __func__,
            "Failed to create compute pipeline state."
        );
        throw std::runtime_error("Failed to create compute pipeline state.");
    }
}

void GltfModelSystem::_CreateRootSignatureCS()
{
    RootParameters rootParametersCS;
    rootParametersCS
        .SetParameter(0, "t0", D3D12_SHADER_VISIBILITY_ALL)
        .SetParameter(1, "t1", D3D12_SHADER_VISIBILITY_ALL)
        .SetParameter(2, "t2", D3D12_SHADER_VISIBILITY_ALL)
        .SetParameter(3, "u0", D3D12_SHADER_VISIBILITY_ALL)
        .SetParameter(4, "b0", D3D12_SHADER_VISIBILITY_ALL);

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.pParameters = rootParametersCS.BuildParams();
    rootSignatureDesc.NumParameters = static_cast<UINT>(rootParametersCS.GetSize());
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    rootSignatureDesc.pStaticSamplers = nullptr; // CSではStaticSamplerは使用しない
    rootSignatureDesc.NumStaticSamplers = 0;

    // シリアライズしてバイナリにする
    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr))
    {
        Logger::GetInstance()->LogError(
            "Object3dSystem",
            __func__,
            reinterpret_cast<char*>(errorBlob->GetBufferPointer())
        );

        assert(false);
    }
    // バイナリをもとに生成
    hr = pDx12_->GetDevice()->CreateRootSignature(
        0, 
        signatureBlob->GetBufferPointer(), 
        signatureBlob->GetBufferSize(), 
        IID_PPV_ARGS(&rootSignature_)
    );
    assert(SUCCEEDED(hr));
}