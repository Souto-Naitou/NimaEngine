#include "NimaFramework.h"
#include <clocale>

#include <NiGui/NiGui.h>

void NimaFramework::Run()
{
    setlocale(LC_ALL, "ja_JP.Utf-8");

    /// 初期化
    Initialize();

    while (true)
    {
        /// 更新
        Update();

        /// 終了判定
        if (IsExitProgram())
        {
            break;
        }

        /// 描画
        Draw();
    }

    Finalize();
}

void NimaFramework::Initialize()
{
    /// システムクラスの初期化
    pConfigManager_ = ConfigManager::GetInstance();
    pLogger_ = Logger::GetInstance();
    pDirectX_ = std::make_unique<DirectX12>();

    pDebugManager_ = DebugManager::GetInstance();
    pWinSystem_ = WinSystem::GetInstance();
    pSpriteSystem_ = SpriteSystem::GetInstance();
    pObject3dSystem_ = Object3dSystem::GetInstance();
    pParticleSystem_ = ParticleSystem::GetInstance();
    pTextureManager_ = TextureManager::GetInstance();
    pSRVManager_ = SRVManager::GetInstance();
    pSceneManager_ = SceneManager::GetInstance();
    pParticleStorage_ = ParticleStorage::GetInstance();
    pLineSystem_ = LineSystem::GetInstance();
    pInput_ = Input::GetInstance();
    pRandomGenerator_ = RandomGenerator::GetInstance();
    pTextSystem_ = TextSystem::GetInstance();
    pAudioManager_ = AudioManager::GetInstance();
    pEventTimer_ = EventTimer::GetInstance();

    #ifdef _DEBUG
    pImGuiManager_ = std::make_unique<ImGuiManager>();
    #endif // _DEBUG

    /// ロガーの初期化
    pLogger_->Initialize();
    pDebugManager_->SetLoggerWindow(pLogger_);

    // 設定ファイルの読み込み
    pConfigManager_->Initialize("resources/json/.engine/config.json");


    /// ウィンドウの初期化
    pWinSystem_->Initialize();
    pWinSystem_->ShowWnd();

    /// DirectX12の初期化
    if (pWinSystem_->IsResized()) pDirectX_->Initialize();

    /// SRVManagerの初期化
    pSRVManager_->Initialize(pDirectX_.get());

    /// ImGui基盤の初期化
    #ifdef _DEBUG
    pImGuiManager_->SetDirectX12(pDirectX_.get());
    pImGuiManager_->Initialize();
    #endif // _DEBUG

    // デバッグマネージャの初期化
    pDebugManager_->SetDirectX12(pDirectX_.get());

    /// テクスチャマネージャの初期化
    pTextureManager_->SetDirectX12(pDirectX_.get());
    pTextureManager_->Initialize(pSRVManager_);

    /// スプライト基盤の初期化
    pSpriteSystem_->SetDirectX12(pDirectX_.get());
    pSpriteSystem_->Initialize();

    /// 3Dオブジェクト基盤の初期化
    this->InitializeObject3dSystem();

    /// パーティクル基盤の初期化
    pParticleSystem_->SetDirectX12(pDirectX_.get());
    pParticleSystem_->Initialize();

    /// ライン基盤の初期化
    pLineSystem_->SetDirectX12(pDirectX_.get());
    pLineSystem_->Initialize();

    /// テキスト基盤の初期化
    pTextSystem_->SetDirectX12(pDirectX_.get());
    pTextSystem_->Initialize();

    /// オーディオの初期化
    pAudioManager_->Initialize();

    /// イベントタイマーをデバッグマネージャに設定
    pDebugManager_->SetEventTimerWindow(pEventTimer_);

    /// 入力の初期化
    pInput_->Initialize(GetModuleHandleA(nullptr), pWinSystem_->GetHwnd());

    /// 乱数生成器の初期化
    pRandomGenerator_->Initialize();

    /// SRVが初期化された後に呼ぶ
    pDirectX_->CreateGameScreenResource();

    /// ビューポートの初期化
    pViewport_ = std::make_unique<Viewport>();
    pViewport_->SetDirectX12(pDirectX_.get());
    pViewport_->Initialize();
    pTextSystem_->SetViewport(pViewport_.get());
    pDebugManager_->SetViewportWindow(pViewport_.get());

    /// レイヤーの初期化
    pLayer_ = std::make_unique<Layer>();

    /// シーンマネージャの初期化
    SceneManager::Params sceneManagerParams;
    sceneManagerParams.pDx12 = pDirectX_.get();
    sceneManagerParams.pLayer = pLayer_.get();
    #ifdef _DEBUG
    sceneManagerParams.pImGuiManager = pImGuiManager_.get();
    #endif // _DEBUG

    pSceneManager_->Initialize(sceneManagerParams);

    pParticleStorage_->SetDirectX12(pDirectX_.get());

    /// UIの初期化
    D3D12_VIEWPORT vp = pDirectX_->GetViewport();
    NiGui::Initialize({ vp.Width, vp.Height }, { vp.TopLeftX, vp.TopLeftY });
    NiGui::SetClientSize({WinSystem::clientWidth, WinSystem::clientHeight});
    

    NiGui::SetConfirmSound(pAudioManager_->GetNewAudio("UI", "ui_confirm.wav"));
    NiGui::SetHoverSound(pAudioManager_->GetNewAudio("UI", "ui_hover.wav"));

    /// Drawerの設定
    pDrawer_ = std::make_unique<NiGuiDrawer>();
    NiGui::SetDrawer(pDrawer_.get());

    /// デバッグUIの設定
    auto& io = NiGui::GetIO();
    auto& state = NiGui::GetState();
    auto& setting = NiGui::GetSetting();
    pNiGuiDebug_ = std::make_unique<NiGuiDebug>();
    pNiGuiDebug_->SetIO(&io);
    pNiGuiDebug_->SetState(&state);
    pNiGuiDebug_->SetSetting(&setting);
   
    NiGui::SetDebug(pNiGuiDebug_.get());

    /// Gltfモデルシステムの初期化
    pGltfModelSystem_ = std::make_unique<GltfModelSystem>();
    pGltfModelSystem_->SetDirectX12(pDirectX_.get());
    pGltfModelSystem_->Initialize();
    
    // Cubemapシステムの初期化
    pCubemapSystem_ = std::make_unique<CubemapSystem>();
    pCubemapSystem_->SetDirectX12(pDirectX_.get());
    pCubemapSystem_->Initialize();

    /// コマンドリストを追加
    pDirectX_->AddCommandList(DirectX12::CommandListType::DrawableObject, pObject3dSystem_->GetCommandList());
    pDirectX_->AddCommandList(DirectX12::CommandListType::DrawableObject, pParticleSystem_->GetCommandList());
    pDirectX_->AddCommandList(DirectX12::CommandListType::DrawableObject, pSpriteSystem_->GetCommandList());
    pDirectX_->AddCommandList(DirectX12::CommandListType::DrawableObject, pLineSystem_->GetCommandList());

    /// デフォルトシーン引数の設定
    (*pSceneManager_)
        .AddInitialArg("DirectX12", pDirectX_.get())
        #ifdef _DEBUG
        .AddInitialArg("ImGuiManager", pImGuiManager_.get())
        #endif // _DEBUG
        .AddInitialArg("Object3dSystem", pObject3dSystem_)
        .AddInitialArg("ParticleSystem", pParticleSystem_)
        .AddInitialArg("SpriteSystem", pSpriteSystem_)
        .AddInitialArg("LineSystem", pLineSystem_)
        .AddInitialArg("ParticleManager", pParticleStorage_)
        .AddInitialArg("AudioManager", pAudioManager_)
        .AddInitialArg("GltfModelSystem", pGltfModelSystem_.get())
        .AddInitialArg("CubemapSystem", pCubemapSystem_.get())
        .AddInitialArg("Layer", pLayer_.get());

    pDirectX_->AddOnResizeAfter("Viewport", std::bind(&Viewport::OnResizedBuffers, pViewport_.get()));
    pDirectX_->AddOnResizeAfter("TextSystem", std::bind(&TextSystem::OnResizedBuffers, pTextSystem_));

    #ifdef _DEBUG
    pDirectX_->AddOnResizeAfter("ImGuiManager", std::bind(&ImGuiManager::OnResizedBuffers, pImGuiManager_.get()));
    #endif // _DEBUG
}

void NimaFramework::Finalize()
{
    pSceneManager_->Finalize();
    pParticleStorage_->Finalize();
    pAudioManager_->Finalize();

    #ifdef _DEBUG
    pImGuiManager_->Finalize();
    #endif // _DEBUG

    pWinSystem_->Finalize();
    pLogger_->Save();
}

void NimaFramework::Update()
{
    /// イベント計測開始
    #ifdef _DEBUG
    pEventTimer_->NewFrame();
    pEventTimer_->BeginEvent("Update");
    #endif // _DEBUG

    UINT msg = pWinSystem_->GetMsg();
    if (msg == WM_QUIT)
    {
        isExitProgram_ = true;
        return;
    }

    if(pWinSystem_->IsResized())
    {
        // ウィンドウのリサイズ後、バッファーのリサイズ前
        pTextSystem_->OnResizedWindow();
        pDirectX_->OnResizedWindow();
    }

    #ifdef _DEBUG

    /// UIの更新
    NiGui::SetWindowInfo(
        { pViewport_->GetViewportSize().x, pViewport_->GetViewportSize().y },
        { pViewport_->GetViewportPos().x, pViewport_->GetViewportPos().y }
    );

    #endif // _DEBUG

    NiGui::BeginFrame();

    /// マネージャ更新
    pInput_->Update();
    pAudioManager_->Update();

    #ifdef _DEBUG
    pImGuiManager_->BeginFrame();
    #endif // _DEBUG

    /// シーン更新
    pSceneManager_->Update();
    pDebugManager_->Update();

    /// イベント計測終了
    #ifdef _DEBUG
    pEventTimer_->EndEvent("Update");
    #endif // _DEBUG

    /// パーティクル更新
    pParticleStorage_->Update();
}

void NimaFramework::Draw()
{
    /// イベント計測開始
    pEventTimer_->BeginEvent("Draw");

    // シーンの描画関数呼び出し
    pSceneManager_->SceneDraw();

    /// 前景スプライトの描画
    NiGui::DrawUI();

    // Canvasに登録されているオブジェクトをCanvasに描画する
    pLayer_->DrawObjects();

    pObject3dSystem_->DrawCall();
    pSpriteSystem_->DrawCall();
    pParticleSystem_->DrawCall();

    // 同期待ち
    pObject3dSystem_->Sync();
    pSpriteSystem_->Sync();
    pParticleSystem_->Sync();

    // Canvasにポストエフェクトを適用する
    pLayer_->ApplyPostEffects();

    // レンダーターゲットの初期化
    pDirectX_->NewFrame();

    // エフェクト適用後のCanvasを描画
    pLayer_->DrawCanvases();

    // レンダーターゲットからビューポート用リソースにコピー (Releaseでは実行されない)
    pDirectX_->CopyFromRTV(pDirectX_->GetCommandListLast());

    // コンピュートシェーダーの実行
    pViewport_->Compute();

    /// イベント計測終了と出力
    pEventTimer_->EndEvent("Draw");
    pEventTimer_->EndFrame();

    pDebugManager_->DrawUI();

    /// ImGuiの描画
    #ifdef _DEBUG
    if (pDebugManager_->IsDisplay())
    {
        NiGui::DrawDebug();
    }
    pImGuiManager_->Render();
    #endif // _DEBUG

    /// コマンドの実行
    pDirectX_->CommandExecute();

    /// テキストの描画
    pTextSystem_->PresentDraw();
    pSceneManager_->SceneDrawText();
    pTextSystem_->PostDraw();
}

void NimaFramework::PreProcess()
{
    pSRVManager_->SetDescriptorHeaps();
    pLayer_->PreDraw();
    /// レンダーターゲットビューのハンドルを各システムに設定
    uint32_t indexBackbuffer = pDirectX_->GetBackBufferIndex();
    auto rtvHandleSwapChain_ = pDirectX_->GetRTVHandle()[indexBackbuffer];
    pObject3dSystem_->SetRTVHandle(&rtvHandleSwapChain_);
    pSpriteSystem_->SetRTVHandle(&rtvHandleSwapChain_);
    pParticleSystem_->SetRTVHandle(&rtvHandleSwapChain_);
    pLineSystem_->SetRTVHandle(&rtvHandleSwapChain_);
}

void NimaFramework::PostProcess()
{
    pDirectX_->DisplayFrame();
    pObject3dSystem_->PostDraw();
    pSpriteSystem_->PostDraw();
    pParticleSystem_->PostDraw();
    pLineSystem_->PostDraw();
    pTextureManager_->ReleaseIntermediateResources();
    pLayer_->PostDraw();
    pViewport_->PostDraw();
    #ifdef _DEBUG
    pImGuiManager_->PostDraw();
    #endif // _DEBUG
}

void NimaFramework::InitializeObject3dSystem()
{
    pObject3dSystem_->SetDirectX12(pDirectX_.get());
    pObject3dSystem_->Initialize();

    // デフォルトの環境テクスチャを読み込み
    pTextureManager_->LoadTexture(kEnvTexturePathDefault);
    auto envTexture = pTextureManager_->GetSrvHandleGPU(kEnvTexturePathDefault);

    // 環境テクスチャを設定
    pObject3dSystem_->SetEnvironmentTexture(envTexture);
}
