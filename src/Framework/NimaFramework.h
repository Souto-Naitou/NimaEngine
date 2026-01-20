#pragma once

#include <Core/ConfigManager/ConfigManager.h>
#include <Core/Window/Window.h>
#include <Core/DirectX12/TextureManager.h>
#include <Core/DirectX12/DirectX12.h>
#include <Core/DirectX12/SRVManager.h>
#include <Core/DirectX12/PipelineStateObject/PSOCache.h>
#include <Core/DirectX12/RootSignature/RootSignatureCache.h>
#include <Features/Audio/AudioManager.h>
#include <Features/Input/Input.h>
#include <Features/Model/ModelManager.h>
#include <Features/SceneManager/SceneManager.h>
#include <Features/Line/LineSystem.h>
#include <Features/RandomGenerator/RandomGenerator.h>
#include <Features/Text/TextSystem.h>
#include <Features/Viewport/Viewport.h>
#include <Features/NiGui/NiGuiDrawer.h>
#include <Features/NiGui/NiGuiDebug.h>
#include <Features/Cubemap/CubemapSystem.h>
#include <Features/Layer/OrderedCanvasLayer.h>
#include <Features/Model/GltfModelSystem.h>
#include <Features/Event/EventListener.h>
#include <drawable/sprite/SpriteSystem.h>
#include <drawable/object3d/Object3dSystem.h>
#include <drawable/particle/ParticleStorage.h>
#include <drawable/particle/ParticleSystem.h>
#include <DebugTools/EventTimer/EventTimer.h>
#include <DebugTools/Logger/Logger.h>
#include <DebugTools/DebugManager/DebugManager.h>
#include <DebugTools/ImGuiManager/ImGuiManager.h>
#include <Interfaces/ISceneFactory.h>
#include <Effects/PostEffects/GlobalInput/PostEffectInputCommon.h>

#include <memory> /// std::unique_ptr
#include <Core/DirectX12/PostEffectExecutor.h>


/// ゲーム共通のフレームワーククラス
class NimaFramework
{
public:
    /// <summary>
    /// フレームワークのメインループを開始します。
    /// </summary>
    void                            Run();


public:
    virtual                         ~NimaFramework() {}

    /// <summary>
    /// システムとサブシステムの初期化を行います。
    /// </summary>
    virtual void                    Initialize();
    /// <summary>
    /// 終了処理を行います。
    /// </summary>
    virtual void                    Finalize();
    /// <summary>
    /// フレームごとの更新処理を行います。
    /// </summary>
    virtual void                    Update();
    /// <summary>
    /// フレームごとの描画処理を行います。
    /// </summary>
    virtual void                    Draw();
    virtual bool                    IsExitProgram() const { return isExitProgram_; }
    /// <summary>
    /// 描画前の共通前処理を行います。
    /// </summary>
    void                            PreProcess();
    /// <summary>
    /// 描画後の共通後処理を行います。
    /// </summary>
    void                            PostProcess();


protected: 
    static constexpr char kEnvTexturePathDefault[] = "EngineResources/Textures/white32x32.dds";

    /// システムクラスのインスタンス
    std::unique_ptr<DirectX12>                  pDirectX_                   = nullptr;
    std::unique_ptr<ISceneFactory>              pSceneFactory_              = nullptr;
    std::unique_ptr<Viewport>                   pViewport_                  = nullptr;
    std::unique_ptr<NiGuiDrawer>                pDrawer_                    = nullptr;
    std::unique_ptr<NiGuiDebug>                 pNiGuiDebug_                = nullptr;
    std::unique_ptr<GltfModelSystem>            pGltfModelSystem_           = nullptr;
    std::unique_ptr<CubemapSystem>              pCubemapSystem_             = nullptr;
    std::unique_ptr<OrderedCanvasLayer>         pLayer_                     = nullptr;
    std::unique_ptr<SceneTransitionExecutor>    pTransitionExecutor_        = nullptr;

    #ifdef _DEBUG
    std::unique_ptr<ImGuiManager>   pImGuiManager_              = nullptr;
    #endif // _DEBUG

    /// 他クラスのインスタンス
    ConfigManager*                  pConfigManager_             = nullptr;
    Logger*                         pLogger_                    = nullptr;
    DebugManager*                   pDebugManager_              = nullptr;
    Window*                         pWinSystem_                 = nullptr;
    SRVManager*                     pSRVManager_                = nullptr;
    TextureManager*                 pTextureManager_            = nullptr;
    SceneManager*                   pSceneManager_              = nullptr;
    SpriteSystem*                   pSpriteSystem_              = nullptr;
    Object3dSystem*                 pObject3dSystem_            = nullptr;
    ParticleSystem*                 pParticleSystem_            = nullptr;
    ParticleStorage*                pParticleStorage_           = nullptr;
    LineSystem*                     pLineSystem_                = nullptr;
    TextSystem*                     pTextSystem_                = nullptr;
    Input*                          pInput_                     = nullptr;
    RandomGenerator*                pRandomGenerator_           = nullptr;
    AudioManager*                   pAudioManager_              = nullptr;
    EventTimer*                     pEventTimer_                = nullptr;
    EventListener*                  pEventListener_             = nullptr;
    PSOCache*                       pPSOCache_                  = nullptr;
    RootSignatureCache*             pRootSignatureCache_        = nullptr;
    PostEffectInputCommon*          pPostEffectInputCommon_     = nullptr;

    bool                            isExitProgram_              = false;

private:
    void InitializeObject3dSystem();
};

#define CREATE_APPLICATION(class) \
int _stdcall WinMain(HINSTANCE, HINSTANCE, LPSTR, int) \
{ \
    std::unique_ptr<NimaFramework> program = std::make_unique<class>(); \
    program->Run(); \
    return 0; \
}

#define CREATE_FRAMEWORK(classname) \
\
class classname : public NimaFramework \
{ \
public: \
    void Initialize() override; \
    void Finalize() override; \
    void Update() override; \
    void Draw() override; \
    bool IsExitProgram() { return isExitProgram_; } \
private: \
};