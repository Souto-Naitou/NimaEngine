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
    pPostEffectExecuter_ = std::make_unique<PostEffectExecuter>();

    pDebugManager_ = DebugManager::GetInstance();
    pWinSystem_ = WinSystem::GetInstance();
    pSpriteSystem_ = SpriteSystem::GetInstance();
    pObject3dSystem_ = Object3dSystem::GetInstance();
    pParticleSystem_ = ParticleSystem::GetInstance();
    pTextureManager_ = TextureManager::GetInstance();
    pSRVManager_ = SRVManager::GetInstance();
    pSceneManager_ = SceneManager::GetInstance();
    pParticleManager_ = ParticleManager::GetInstance();
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

    /// シーンマネージャの初期化
    pSceneManager_->Initialize();

    pParticleManager_->SetDirectX12(pDirectX_.get());

    /// UIの初期化
    auto vp = pDirectX_->GetViewport();
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

    /// ポストエフェクトの初期化
    pPostEffectExecuter_->Initialize(pDirectX_.get());

    /// コマンドリストを追加
    pDirectX_->AddCommandList(pObject3dSystem_->GetCommandList());
    pDirectX_->AddCommandList(pParticleSystem_->GetCommandList());
    pDirectX_->AddCommandList(pSpriteSystem_->GetCommandList());
    pDirectX_->AddCommandList(pLineSystem_->GetCommandList());
    pDirectX_->AddCommandList(pPostEffectExecuter_->GetCommandList());

    pDirectX_->AddOnResize("PostEffect", std::bind(&PostEffectExecuter::OnResize, pPostEffectExecuter_.get()));

    /// デフォルトシーン引数の設定
    (*pSceneManager_)
        .AddInitialArg("DirectX12", pDirectX_.get())
        .AddInitialArg("PostEffectExecuter", pPostEffectExecuter_.get())
        .AddInitialArg("Object3dSystem", pObject3dSystem_)
        .AddInitialArg("ParticleSystem", pParticleSystem_)
        .AddInitialArg("SpriteSystem", pSpriteSystem_)
        .AddInitialArg("LineSystem", pLineSystem_)
        .AddInitialArg("ParticleManager", pParticleManager_)
        .AddInitialArg("AudioManager", pAudioManager_)
        .AddInitialArg("GltfModelSystem", pGltfModelSystem_.get())
        .AddInitialArg("CubemapSystem", pCubemapSystem_.get());
}

void NimaFramework::Finalize()
{
    pDirectX_->DeleteOnResize("PostEffect");
    pAudioManager_->Finalize();
    pWinSystem_->Finalize();
    pLogger_->Save();
    pParticleManager_->Finalize();
    pSceneManager_->Finalize();
    pPostEffectExecuter_->Finalize();

    #ifdef _DEBUG
    pImGuiManager_->Finalize();
    #endif // _DEBUG
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
        // バッファーのリサイズ後
        pPostEffectExecuter_->OnResizedBuffers();
        pViewport_->OnResizedBuffers();
        #ifdef _DEBUG
        pImGuiManager_->OnResizedBuffers();
        #endif // _DEBUG
        pTextSystem_->OnResizedBuffers();
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

    pDebugManager_->Update();
    pDebugManager_->DrawUI();
    pViewport_->DrawWindow();

    #ifdef _DEBUG
    if (pDebugManager_->IsDisplay())
    {
        pLogger_->DrawUI();
        NiGui::DrawDebug();
    }
    #endif // _DEBUG

    /// シーン更新
    pSceneManager_->Update();


    /// イベント計測終了
    #ifdef _DEBUG
    pEventTimer_->EndEvent("Update");
    #endif // _DEBUG

    /// パーティクル更新
    pParticleManager_->Update();
}

void NimaFramework::Draw()
{
    /// イベント計測開始
    pEventTimer_->BeginEvent("Draw");

    /// パーティクル描画
    pParticleManager_->Draw();
    pParticleSystem_->DrawCall();

    // シーンの描画 (テキスト以外)
    pSceneManager_->SceneDraw();
    pObject3dSystem_->DrawCall();

    /// 前景スプライトの描画
    NiGui::DrawUI();
    pSpriteSystem_->DrawCall();

    // Canvasに登録されているオブジェクトをCanvasに描画する
    // pLayer_->DrawObjects();

    // 同期待ち
    pObject3dSystem_->Sync();
    pParticleSystem_->Sync();
    pSpriteSystem_->Sync();

    // Canvasにポストエフェクトを適用する
    // pLayer_->ApplyPostEffects();

    // エフェクト適用後のCanvasを描画
    // pLayer_->DrawCanvases();

    // ポストエフェクトの適用
    pPostEffectExecuter_->ApplyPostEffects();

    // レンダーターゲットの初期化
    pDirectX_->NewFrame();

    // ポストエフェクト後のテクスチャをスワップチェーンリソースに描画
    pPostEffectExecuter_->Draw();

    // レンダーターゲットからビューポート用リソースにコピー (Releaseでは実行されない)
    pDirectX_->CopyFromRTV(pDirectX_->GetCommandListsLast());
    // コンピュートシェーダーの実行
    pViewport_->Compute();

    /// イベント計測終了と出力
    pEventTimer_->EndEvent("Draw");
    pEventTimer_->EndFrame();

    if (pDebugManager_->IsDisplay()) pEventTimer_->ImGui();

    /// ImGuiの描画
    #ifdef _DEBUG
    pImGuiManager_->Render();
    pImGuiManager_->EndFrame();
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
    pPostEffectExecuter_->NewFrame();
    pSRVManager_->SetDescriptorHeaps();
    auto rtvHandle = pPostEffectExecuter_->GetRTVHandle();
    pObject3dSystem_->SetRTVHandle(rtvHandle);
    pSpriteSystem_->SetRTVHandle(rtvHandle);
    pParticleSystem_->SetRTVHandle(rtvHandle);
    pLineSystem_->SetRTVHandle(rtvHandle);
}

void NimaFramework::PostProcess()
{
    pDirectX_->DisplayFrame();
    pObject3dSystem_->PostDraw();
    pSpriteSystem_->PostDraw();
    pParticleSystem_->PostDraw();
    pLineSystem_->PostDraw();
    pPostEffectExecuter_->PostDraw();
    pTextureManager_->ReleaseIntermediateResources();
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
