#include "PostEffectExecuter.h"

#include <Core/DirectX12/Helper/DX12Helper.h>
#include <Core/Win32/WinSystem.h>
#include <Effects/PostEffects/.Helper/PostEffectHelper.h>
#include <DebugTools/DebugManager/DebugManager.h>

#ifdef _DEBUG
#include <imgui.h>
#endif //_DEBUG
#include <config/EngineSetting.h>
#include "BlendDesc.h"

void PostEffectExecuter::Initialize(DirectX12* pDx12, DX12Resource* pResource, bool isRegisterDebugWindow)
{
    pDx12_ = pDx12;

    pRenderTexture_ = pResource;

    // インスタンスの取得
    ObtainInstances();

    // 描画用コマンドリストの生成
    CreateCommandList();

    // ルートシグネチャの生成
    CreateRootSignature();

    // パイプラインステートの生成
    CreatePipelineState();

    // デバッグウィンドウの登録
    #ifdef _DEBUG
    if (isRegisterDebugWindow)
    {
        pDebugEntry_ = std::make_unique<DebugEntry<PostEffectExecuter>>("PostEffect", "EffectList", this);
    }
    #else
    isRegisterDebugWindow;
    #endif //_DEBUG

    pEffectFactory_ = std::make_unique<PostEffectFactory>(pDx12_, commandListForDraw_.Get());
}

void PostEffectExecuter::Finalize()
{
    pDx12_->RemoveCommandList(DirectX12::CommandListType::PostEffectExecuter, commandListForDraw_.Get());
}

void PostEffectExecuter::ApplyPostEffects()
{
    pRenderTexture_->GetStateTracker().ChangeState(commandListForDraw_.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    outputHandleGpu_ = pRenderTexture_->GetSRVHandleGPU();

    for (auto it = postEffects_.begin(); it != postEffects_.end(); ++it)
    {
        auto postEffect = it->get();

        if (!postEffect->Enabled()) continue;

        // 適用前テクスチャの設定
        postEffect->SetInputTextureHandle(outputHandleGpu_);

        // 1. PostEffectの設定 (PSOなど)
        postEffect->Setting();
        // 2. PostEffectの描画
        postEffect->Apply();
        // 3. PostEffectの描画後の処理
        postEffect->ToShaderResourceState();
        outputHandleGpu_ = postEffect->GetOutputTextureHandle();
    }
}

void PostEffectExecuter::Draw()
{
    uint32_t indexBuckbuffer = pDx12_->GetBackBufferIndex();
    auto rtvHandle = pDx12_->GetRTVHandle()[indexBuckbuffer];
    commandListForDraw_->OMSetRenderTargets(1, &rtvHandle, false, nullptr);
    // レンダーターゲットがSwapchainリソースになっている前提
    commandListForDraw_->SetGraphicsRootSignature(rootSignature_.Get());
    commandListForDraw_->SetPipelineState(pso_.Get());
    commandListForDraw_->SetGraphicsRootDescriptorTable(0, outputHandleGpu_);
    commandListForDraw_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandListForDraw_->DrawInstanced(3, 1, 0, 0);

    pRenderTexture_->GetStateTracker().ChangeState(commandListForDraw_.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
}

void PostEffectExecuter::PreDraw()
{
    // コマンドリストの設定
    uint32_t indexBackbuffer = pDx12_->GetBackBufferIndex();
    rtvHandleSwapChain_ = pDx12_->GetRTVHandle()[indexBackbuffer];
    DX12Helper::CommandListCommonSetting(pDx12_, commandListForDraw_.Get(), &rtvHandleSwapChain_);

    // Object3dやSpriteの描画先を決定する関数

    /// 描画先のRTV/DSVの設定
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvHeap_->GetCPUDescriptorHandleForHeapStart();
    commandListMain_->OMSetRenderTargets(1, &pRenderTexture_->GetRTVHandle(), false, &dsvHandle);

    // 画面全体のクリア
    commandListMain_->ClearRenderTargetView(pRenderTexture_->GetRTVHandle(), &NimaEngine::Config::kEditorBGColor.x, 0, nullptr);

    // 指定した深度で画面全体をクリア
    commandListMain_->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // 描画先のビューを設定
    commandListMain_->RSSetViewports(1, &pDx12_->GetViewport());
    commandListMain_->RSSetScissorRects(1, &pDx12_->GetScissorRect());
}

void PostEffectExecuter::PostDraw()
{
    commandAllocator_->Reset();
    commandListForDraw_->Reset(commandAllocator_.Get(), nullptr);
}

void PostEffectExecuter::OnResizeBefore()
{
    for (auto& posteffect : postEffects_) posteffect->OnResizeBefore();
}

void PostEffectExecuter::OnResizeAfter()
{
    for (auto& posteffect : postEffects_) posteffect->OnResizeAfter();
}

void PostEffectExecuter::ImGui()
{
    #ifdef _DEBUG

    // staticな変数で状態を保持
    static const ImVec4 kColorRed(1.0f, 0.0f, 0.0f, 1.0f);
    static const ImVec4 kColorGreen(0.0f, 1.0f, 0.0f, 1.0f);

    bool isBeginTable = ImGui::BeginTable("PostEffectTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg);

    if (isBeginTable)
    {
        ImGui::TableSetupColumn("Effect Name"       , ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn(" Enable "          , ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("  Solo  "          , ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableHeadersRow();

        // 一列目の処理
        auto fnColumn1 = [&](int i) -> void
        {
            std::string name = postEffects_[i]->GetName();
            // Selectableで要素を表示・選択状態を管理
            if (ImGui::Selectable(name.c_str(), selectedIndex_ == i))
            {
                if (selectedIndex_ == i) selectedIndex_ = -1;
                else selectedIndex_ = i;
            }
            if (ImGui::BeginPopupContextItem()) // <-- use last item id as popup id
            {
                selectedIndex_ = i;
                postEffects_[selectedIndex_]->DebugOverlay();

                if (ImGui::Button("Close"))
                    ImGui::CloseCurrentPopup();
                ImGui::EndPopup();
            }
            ImGui::SetItemTooltip("Right-click to open setting");
        };

        // 二列目の処理
        auto fnColumn2 = [&](int i) -> void
        {
            auto fn = [&]() -> void
            {
                if (postEffects_[i]->Enabled()) ImGui::TextColored(kColorGreen, "Yes");
                else ImGui::TextColored(kColorRed, "No");
            };

            this->ImGuiCenterTable(fn);
        };

        // 三列目の処理
        auto fnColumn3 = [&](int i) -> void
        {
            auto fn = [&](int i) -> void
            {
                bool isSolo = (soloIndex_ == i);
                if (ImGui::SmallButton("S"))
                {
                    isSolo = !isSolo; // ボタンが押されたらソロモードの切り替え
                    if (isSolo)
                    {
                        soloIndex_ = i;
                        this->EnableSolo(i); // ソロモードを有効化
                    }
                    else if (soloIndex_ == i)
                    {
                        postEffects_[i]->Enable(false); // チェックを外すとソロ解除
                        soloIndex_ = -1; // チェックを外すとソロ解除
                    }
                }
            };

            this->ImGuiCenterTable(std::bind(fn, i));
        };

        // テーブル構成
        for (int i = 0; i < postEffects_.size(); ++i)
        {
            ImGui::PushID(i); // 各要素にユニークなIDを付与

            ImGui::TableNextColumn(); // 次の列へ移動

            fnColumn1(i);

            ImGui::TableNextColumn(); // 2列目へ移動

            fnColumn2(i);

            ImGui::TableNextColumn(); // 3列目へ移動

            fnColumn3(i);

            ImGui::PopID(); // IDをポップして元に戻す
        }

        ImGui::EndTable(); // テーブルの終了
    }

    ImGui::Spacing();

    // 移動ボタンの表示と操作
    bool isEnable = false;
    if (selectedIndex_ < 0)
    {
        ImGui::BeginDisabled();
    }
    else
    {
        isEnable = postEffects_[selectedIndex_]->Enabled();
    }

    if (ImGui::Button("Up") && selectedIndex_ > 0)
    {
        std::swap(postEffects_[selectedIndex_], postEffects_[selectedIndex_ - 1]);
        selectedIndex_--;  // 選択インデックスも一緒に更新
    }

    ImGui::SameLine();

    if (ImGui::Button("Down") && selectedIndex_ < postEffects_.size() - 1)
    {
        std::swap(postEffects_[selectedIndex_], postEffects_[selectedIndex_ + 1]);
        selectedIndex_++;  // 選択インデックスも更新
    }

    ImGui::SameLine();

    if (ImGui::Checkbox("Enabled", &isEnable))
    {
        postEffects_[selectedIndex_]->Enable(isEnable);
    }

    if (selectedIndex_ < 0) 
    {
        ImGui::EndDisabled();
        ImGui::Text("項目を選択してください");
    }

    #endif //_DEBUG
}

IPostEffect* PostEffectExecuter::AddEffect(PostEffectClassName name)
{
    auto effect = pEffectFactory_->CreatePostEffect(name);
    PostEffectInitParams initParams{};
    initParams.pDx12 = pDx12_;
    initParams.pCommandList = commandListForDraw_.Get();
    effect->Initialize(initParams);

    postEffects_.push_back(std::move(effect));
    return postEffects_.back().get();
}

bool PostEffectExecuter::RemoveEffect(IPostEffect* pEffect)
{
    auto it = std::find_if(
        postEffects_.begin(), 
        postEffects_.end(),
        [pEffect](const std::unique_ptr<IPostEffect>& effectPtr)
        {
            return effectPtr.get() == pEffect;
        }
    );

    if (it != postEffects_.end())
    {
        postEffects_.erase(it);
        return true;
    }
    return false;
}

void PostEffectExecuter::ObtainInstances()
{
    /// 必要なインスタンスを取得
    rtvHeapCounter_ = pDx12_->GetRTVHeapCounter();
    pDevice_ = pDx12_->GetDevice();
    pSRVManager_ = SRVManager::GetInstance();
    commandListMain_ = pDx12_->GetCommandList();
    dsvHeap_ = pDx12_->GetDSVDescriptorHeap();
    rtvHeap_ = pDx12_->GetRTVDescriptorHeap();
}

void PostEffectExecuter::CreateRootSignature()
{
    D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
    descriptorRange[0].BaseShaderRegister = 0; // 0から始まる
    descriptorRange[0].NumDescriptors = 1; // 数は1つ
    descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRVを使う
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // Offsetを自動計算

    /// RootSignature作成
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    // RootParameter作成。複数設定できるので配列
    D3D12_ROOT_PARAMETER rootParameters[1] = {};
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;       // DescriptorTableを使う
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;                 // PixelShaderで使う
    rootParameters[0].DescriptorTable.pDescriptorRanges = descriptorRange;              // Tableの中身の配列を指定
    rootParameters[0].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);  // Tableで利用する数


    descriptionRootSignature.pParameters = rootParameters;                              // ルートパラメータ配列へのポインタ
    descriptionRootSignature.NumParameters = _countof(rootParameters);                  // 配列の長さ

    D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
    staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;              // 異方性フィルタリング
    staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].MipLODBias = 0.0f;                                    // ミップマップのオフセット
    staticSamplers[0].MaxAnisotropy = 16;                                   // 最大異方性
    staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;         // 比較なし
    staticSamplers[0].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE; // ボーダーカラー
    staticSamplers[0].MinLOD = 0.0f;                                        // 最小ミップレベル
    staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;                           // 最大ミップレベル
    staticSamplers[0].ShaderRegister = 0;                                   // サンプラーのレジスタ番号
    staticSamplers[0].RegisterSpace = 0;                                    // レジスタスペース
    staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;     // ピクセルシェーダーで使用

    descriptionRootSignature.pStaticSamplers = staticSamplers;
    descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

    // シリアライズしてバイナリにする
    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr))
    {
        Logger::GetInstance()->LogError(__FILE__, __FUNCTION__, reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }
    // バイナリをもとに生成
    hr = pDevice_->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
    assert(SUCCEEDED(hr));

}

void PostEffectExecuter::CreatePipelineState()
{
    ID3D12Device* device = pDx12_->GetDevice();
    IDxcUtils* dxcUtils = pDx12_->GetDxcUtils();
    IDxcCompiler3* dxcCompiler = pDx12_->GetDxcCompiler();
    IDxcIncludeHandler* includeHandler = pDx12_->GetIncludeHandler();

    inputLayoutDesc_.pInputElementDescs = nullptr;
    inputLayoutDesc_.NumElements = 0;

    /// BlendStateの設定
    BlendDesc blendDesc = {};
    blendDesc.Initialize(BlendDesc::BlendModes::Alpha);


    /// RasterizerStateの設定
    rasterizerDesc_.CullMode = D3D12_CULL_MODE_NONE;
    rasterizerDesc_.FillMode = D3D12_FILL_MODE_SOLID;
    rasterizerDesc_.MultisampleEnable = TRUE;  // アンチエイリアス有効化
    rasterizerDesc_.AntialiasedLineEnable = TRUE;  // ラインのアンチエイリアス有効化

    /// ShaderをCompileする
    vertexShaderBlob_ = DX12Helper::CompileShader(kVertexShaderPath, L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
    assert(vertexShaderBlob_ != nullptr);

    pixelShaderBlob_ = DX12Helper::CompileShader(kPixelShaderPath, L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);
    assert(pixelShaderBlob_ != nullptr);

    /// PSOを生成する
    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
    graphicsPipelineStateDesc.pRootSignature = rootSignature_.Get();    // RootSignature
    graphicsPipelineStateDesc.InputLayout = inputLayoutDesc_;    // InputLayout
    graphicsPipelineStateDesc.VS = { vertexShaderBlob_.Get()->GetBufferPointer(), vertexShaderBlob_.Get()->GetBufferSize() };
    graphicsPipelineStateDesc.PS = { pixelShaderBlob_.Get()->GetBufferPointer(), pixelShaderBlob_.Get()->GetBufferSize() };
    graphicsPipelineStateDesc.BlendState = blendDesc.Get();            // BlendState
    graphicsPipelineStateDesc.RasterizerState = rasterizerDesc_;    // RasterizerState
    // 書き込むRTVの情報
    graphicsPipelineStateDesc.NumRenderTargets = 1;
    graphicsPipelineStateDesc.RTVFormats[0] = NimaEngine::Config::kRenderTargetFormat;
    // 利用するトポロジ（形状）のタイプ。三角形
    graphicsPipelineStateDesc.PrimitiveTopologyType =
        D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    // どのように画面に色を打ち込むかの設定（気にしなくて良い）
    graphicsPipelineStateDesc.SampleDesc.Count = 1;
    graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    // DepthStencilの設定
    graphicsPipelineStateDesc.DepthStencilState = {};
    graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    // 実際に生成
    HRESULT hr = device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&pso_));
    if (FAILED(hr))
    {
        Logger::GetInstance()->LogError(__FILE__, __FUNCTION__, "Failed to create pipeline state");
        assert(false);
    }

    return;
}

void PostEffectExecuter::CreateCommandList()
{
    Helper::CreateCommandList(pDevice_, commandListForDraw_, commandAllocator_);
    pDx12_->AddCommandList(DirectX12::CommandListType::PostEffectExecuter, commandListForDraw_.Get());
}

void PostEffectExecuter::EnableSolo(const size_t _index)
{
    if (_index < postEffects_.size())
    {
        for (size_t i = 0; i < postEffects_.size(); ++i)
        {
            if (i == _index)
            {
                postEffects_[i]->Enable(true);
            }
            else
            {
                postEffects_[i]->Enable(false);
            }
        }
    }
}

void PostEffectExecuter::ImGuiCenterTable(const std::function<void()>& fn)
{
    #ifdef _DEBUG

    if (ImGui::BeginTable("CenterAlignTable", 3))
    {
        ImGui::TableSetupColumn("##left", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("##center", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("##right", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        fn();
        ImGui::TableNextColumn();
        ImGui::EndTable();
    }

    #else

    fn;

    #endif //_DEBUG
}
