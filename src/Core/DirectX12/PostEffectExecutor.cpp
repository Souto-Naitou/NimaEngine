#include "PostEffectExecutor.h"

#include <Core/DirectX12/Helper/DX12Helper.h>
#include <Core/Window/Window.h>
#include <Effects/PostEffects/.Helper/PostEffectHelper.h>
#include <DebugTools/DebugManager/DebugManager.h>

#ifdef _DEBUG
#include <imgui.h>
#include <imgui_stdlib.h>
#endif //_DEBUG
#include <config/EngineSetting.h>
#include "BlendDesc.h"

#ifdef WIN32_LEAN_AND_MEAN
#undef WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <dxcapi.h>

#include <iterator>

void PostEffectExecutor::Initialize(DirectX12* pDx12, DX12Resource* pResource, bool isRegisterDebugWindow)
{
    pDx12_ = pDx12;

    pResourceInput_ = pResource;

    // インスタンスの取得
    ObtainInstances();

    // 描画用コマンドリストの生成
    CreateCommandList();

    // ルートシグネチャの生成
    RegisterRootSignature();

    // パイプラインステートの生成
    RegisterPipelineState();

    // デバッグウィンドウの登録
    #ifdef _DEBUG
    if (isRegisterDebugWindow)
    {
        pDebugEntry_ = std::make_unique<DebugEntry<PostEffectExecutor>>("PostEffect", "EffectList", this);
    }
    #else
    isRegisterDebugWindow;
    #endif //_DEBUG

    pEffectFactory_ = std::make_unique<PostEffectFactory>(pDx12_, commandListForDraw_.Get());
}

void PostEffectExecutor::Finalize()
{
    pDx12_->RemoveCommandList(DirectX12::CommandListType::PostEffectExecutor, commandListForDraw_.Get());
    for (auto& effect : postEffects_)
    {
        effect->Finalize();
    }
}

void PostEffectExecutor::RegisterCommandListToDirectX12(uint32_t order) const
{
    pDx12_->AddCommandList(DirectX12::CommandListType::PostEffectExecutor, commandListForDraw_.Get(), order);
}

void PostEffectExecutor::ApplyPostEffects()
{
    pResourceInput_->GetStateTracker().ChangeState(commandListForDraw_.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    D3D12_GPU_DESCRIPTOR_HANDLE intermediateHandle = pResourceInput_->GetSRVHandleGPU();
    pResourceIntermediate_ = pResourceInput_;

    for (auto it = postEffects_.begin(); it != postEffects_.end(); ++it)
    {
        auto postEffect = it->get();

        if (!postEffect->Enabled()) continue;

        // 適用前テクスチャの設定
        postEffect->SetInputTextureHandle(intermediateHandle);

        // 1. PostEffectの設定 (PSOなど)
        postEffect->Setting();
        // 2. PostEffectの描画
        postEffect->Apply();
        // 3. PostEffectの描画後の処理
        postEffect->ToShaderResourceState();
        pResourceIntermediate_ = postEffect->GetOutputResource();
        intermediateHandle = pResourceIntermediate_->GetSRVHandleGPU();
    }
}

void PostEffectExecutor::Draw(D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle)
{
    uint32_t indexBuckbuffer = pDx12_->GetBackBufferIndex();
    if (rtvHandle.ptr == 0)
    {
        // 指定されていなければSwapchainのRTVを使用する
        rtvHandle = pDx12_->GetRTVHandle()[indexBuckbuffer];
    }

    commandListForDraw_->OMSetRenderTargets(1, &rtvHandle, false, nullptr);
    commandListForDraw_->SetGraphicsRootSignature(rootSignature_.Get());
    commandListForDraw_->SetPipelineState(pso_.Get());
    commandListForDraw_->SetGraphicsRootDescriptorTable(0, pResourceIntermediate_->GetSRVHandleGPU());
    commandListForDraw_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandListForDraw_->DrawInstanced(3, 1, 0, 0);

    pResourceInput_->GetStateTracker().ChangeState(commandListForDraw_.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
}

void PostEffectExecutor::PreDraw()
{
    // コマンドリストの設定
    uint32_t indexBackbuffer = pDx12_->GetBackBufferIndex();
    rtvHandleSwapChain_ = pDx12_->GetRTVHandle()[indexBackbuffer];
    DX12Helper::CommandListCommonSetting(pDx12_, commandListForDraw_.Get(), &rtvHandleSwapChain_);

    // Object3dやSpriteの描画先を決定する関数

    /// 描画先のRTV/DSVの設定
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvHeap_->GetCPUDescriptorHandleForHeapStart();
    commandListMain_->OMSetRenderTargets(1, &pResourceInput_->GetRTVHandle(), false, &dsvHandle);

    // 画面全体のクリア
    commandListMain_->ClearRenderTargetView(pResourceInput_->GetRTVHandle(), &NimaEngine::Config::kEditorBGColor.x, 0, nullptr);

    // 指定した深度で画面全体をクリア
    commandListMain_->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // 描画先のビューを設定
    commandListMain_->RSSetViewports(1, &pDx12_->GetViewport());
    commandListMain_->RSSetScissorRects(1, &pDx12_->GetScissorRect());
}

void PostEffectExecutor::PostDraw()
{
    commandAllocator_->Reset();
    commandListForDraw_->Reset(commandAllocator_.Get(), nullptr);
}

void PostEffectExecutor::ImGui()
{
    #ifdef _DEBUG

    // staticな変数で状態を保持
    static constexpr ImVec4 kColorRed(1.0f, 0.0f, 0.0f, 1.0f);
    static constexpr ImVec4 kColorGreen(0.0f, 1.0f, 0.0f, 1.0f);

    if (currentSelectedEffect_ == PostEffectClassName::None) ImGui::BeginDisabled();
    if (ImGui::Button("Add"))
    {
        this->AddEffect(currentSelectedEffect_);
    }
    if (currentSelectedEffect_ == PostEffectClassName::None) ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::BeginCombo("##EffectNames", PostEffectFactory::nameMap_.at(currentSelectedEffect_).c_str()))
    {
        for (auto& name : PostEffectFactory::nameMap_)
        {
            const bool isSelected = (currentSelectedEffect_ == name.first);
            if (ImGui::Selectable(name.second.c_str(), isSelected))
            {
                currentSelectedEffect_ = name.first;
            }

            if (isSelected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    bool isBeginTable = ImGui::BeginTable("PostEffectTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg);

    if (isBeginTable)
    {
        ImGui::TableSetupColumn("Effect Name"       , ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn(" Enable "          , ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("  Solo  "          , ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableHeadersRow();

        // 一列目の処理
        auto fnColumn1 = [&](int i, IPostEffect* effect) -> void
        {
            std::string name = effect->GetName();
            // Selectableで要素を表示・選択状態を管理
            if (ImGui::Selectable(name.c_str(), selectedIndex_ == i))
            {
                if (selectedIndex_ == i) selectedIndex_ = -1;
                else selectedIndex_ = i;
            }
            if (ImGui::BeginPopupContextItem()) // <-- use last item id as popup id
            {
                selectedIndex_ = i;
                effect->DebugOverlay();

                if (ImGui::Button("Close"))
                    ImGui::CloseCurrentPopup();
                ImGui::EndPopup();
            }
            ImGui::SetItemTooltip("Right-click to open setting");
        };

        // 二列目の処理
        auto fnColumn2 = [&](IPostEffect* effect) -> void
        {
            auto fn = [&]() -> void
            {
                if (effect->Enabled()) ImGui::TextColored(kColorGreen, "Yes");
                else ImGui::TextColored(kColorRed, "No");
            };

            this->ImGuiCenterTable(fn);
        };

        // 三列目の処理
        auto fnColumn3 = [&](int i, IPostEffect* effect) -> void
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
                        effect->Enable(false); // チェックを外すとソロ解除
                        soloIndex_ = -1; // チェックを外すとソロ解除
                    }
                }
            };

            this->ImGuiCenterTable(std::bind(fn, i));
        };

        // テーブル構成
        for (int i = 0; i < postEffects_.size(); ++i)
        {
            // 要素の取得
            IPostEffect* effect = std::next(postEffects_.begin(), i)->get();

            ImGui::PushID(i); // 各要素にユニークなIDを付与

            ImGui::TableNextColumn(); // 次の列へ移動

            fnColumn1(i, effect);

            ImGui::TableNextColumn(); // 2列目へ移動

            fnColumn2(effect);

            ImGui::TableNextColumn(); // 3列目へ移動

            fnColumn3(i, effect);

            ImGui::PopID(); // IDをポップして元に戻す
        }

        ImGui::EndTable(); // テーブルの終了
    }

    ImGui::Spacing();

    // 移動ボタンの表示と操作
    bool isEnable = false;
    bool isSelected = (selectedIndex_ >= 0);
    auto itrSelected = isSelected ? std::next(postEffects_.begin(), selectedIndex_) : postEffects_.end();
    if (!isSelected)
    {
        ImGui::BeginDisabled();
    }
    else
    {
        isEnable = (*itrSelected)->Enabled();
    }

    if (ImGui::Button("Up") && selectedIndex_ > 0)
    {
        auto itrPrevious = std::prev(itrSelected);
        postEffects_.splice(itrPrevious, postEffects_, itrSelected);

        --selectedIndex_;  // 選択インデックスも一緒に更新
    }

    ImGui::SameLine();

    if (ImGui::Button("Down") && selectedIndex_ < postEffects_.size() - 1)
    {
        // list::splice(...)のArg1の手前にArg3を移動させるため
        // 次の次のイテレータを取得しておく
        auto itrNext = std::next(itrSelected);
        auto itrNextNext = std::next(itrNext);
        postEffects_.splice(itrNextNext, postEffects_, itrSelected);
        ++selectedIndex_;  // 選択インデックスも更新
    }

    ImGui::SameLine();

    if (ImGui::Checkbox("Enabled", &isEnable))
    {
        (*itrSelected)->Enable(isEnable);
    }

    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, kColorRed);
    if (ImGui::Button("Remove"))
    {
        // #FIX: 
        postEffects_.erase(itrSelected);
        itrSelected = postEffects_.end();
        selectedIndex_ = -1; // 選択解除
    }
    ImGui::PopStyleColor();

    if (!isSelected) 
    {
        ImGui::EndDisabled();
        ImGui::Text("項目を選択してください");
    }

    #endif //_DEBUG
}

IPostEffect* PostEffectExecutor::AddEffect(PostEffectClassName name)
{
    auto effect = pEffectFactory_->CreatePostEffect(name);
    PostEffectInitParams initParams{};
    initParams.pDx12 = pDx12_;
    initParams.pCommandList = commandListForDraw_.Get();
    effect->Initialize(initParams);

    postEffects_.push_back(std::move(effect));
    return postEffects_.back().get();
}

bool PostEffectExecutor::RemoveEffect(IPostEffect* pEffect)
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

void PostEffectExecutor::ObtainInstances()
{
    /// 必要なインスタンスを取得
    rtvHeapCounter_ = pDx12_->GetRTVHeapCounter();
    pDevice_ = pDx12_->GetDevice();
    pSRVManager_ = SRVManager::GetInstance();
    commandListMain_ = pDx12_->GetCommandList();
    dsvHeap_ = pDx12_->GetDSVDescriptorHeap();
    rtvHeap_ = pDx12_->GetRTVDescriptorHeap();
}

void PostEffectExecutor::RegisterRootSignature()
{
    auto rsCache = RootSignatureCache::GetInstance();
    if (!rsCache->IsExist(kRootSignatureId_))
    {
        /// RootSignature作成
        RootSignatureDesc rootSignatureDesc = {};
        rootSignatureDesc.params
            .SetParameter(0, "t0", D3D12_SHADER_VISIBILITY_PIXEL);
        /// サンプラーの設定
        rootSignatureDesc.staticSamplers
            .PresetPointWrap()
            .SetMaxAnisotropy(16) // 最大異方性を16に設定
            .SetShaderRegister(0) // シェーダーレジスタ番号を0に設定
            .SetRegisterSpace(0); // レジスタスペースを0に設定
        /// ルートシグネチャ記述子を登録
        rsCache->Register(kRootSignatureId_, rootSignatureDesc);
    }

    rootSignature_ = rsCache->GetOrCreate(kRootSignatureId_);
}

void PostEffectExecutor::RegisterPipelineState()
{
    /// PSOを生成する
    auto psoCache = PSOCache::GetInstance();
    if (!psoCache->IsExist(kPSOId_))
    {
        PSODesc desc{};
        desc.vs = kVertexShaderPath;
        desc.ps = kPixelShaderPath;
        desc.rootSignatureID = kRootSignatureId_;
        desc.blendState.Initialize(BlendDesc::BlendModes::Test);
        desc.inputLayoutDesc.pInputElementDescs = nullptr;
        desc.inputLayoutDesc.NumElements = 0;
        desc.primitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        desc.rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
        desc.rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
        desc.rasterizerDesc.MultisampleEnable = TRUE;  // アンチエイリアス有効化
        desc.rasterizerDesc.AntialiasedLineEnable = TRUE;  // ラインのアンチエイリアス有効化
        psoCache->Register(kPSOId_, desc);
    }

    pso_ = psoCache->GetOrCreate(kPSOId_);
}

void PostEffectExecutor::CreateCommandList()
{
    Helper::CreateCommandList(pDevice_, commandListForDraw_, commandAllocator_);
}

void PostEffectExecutor::EnableSolo(const size_t index)
{
    if (index < postEffects_.size())
    {
        auto itr = postEffects_.begin();
        for (size_t i = 0; i < postEffects_.size(); ++i)
        {
            IPostEffect* effect = itr->get();
            if (i == index)
            {
                effect->Enable(true);
            }
            else
            {
                effect->Enable(false);
            }
            ++itr;
        }
    }
}

void PostEffectExecutor::ImGuiCenterTable(const std::function<void()>& fn)
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
