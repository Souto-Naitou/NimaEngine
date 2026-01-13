#pragma once

#include <Scene/SceneBase.h>
#include <Interfaces/ISceneFactory.h>
#include <Interfaces/IIntermediateScreenFactory.h>
#include <Effects/SceneTransition/SceneTransitionExecutor.h>
#include <Features/Model/ModelManager.h>
#include <memory>
#include <Interfaces/ISceneArgs.h>
#include <unordered_map>
#include <DebugTools/ImGuiManager/ImGuiManager.h>
#include <Features/Layer/OrderedCanvasLayer.h>
#include <Core/DirectX12/DirectX12.h>
#include <io/loader/TaskExecutor.h>

/// <summary>
/// シーン保守クラス
/// </summary>
class SceneManager
{
public:
    SceneManager(SceneManager const&) = delete;
    void operator=(SceneManager const&) = delete;
    SceneManager(SceneManager&&) = delete;
    void operator=(SceneManager&&) = delete;

    static SceneManager* GetInstance()
    {
        static SceneManager instance;
        return &instance;
    }


    /// [Setters]

    /// <summary>
    /// シーン生成に使用するファクトリを設定します。
    /// </summary>
    /// <param name="pSceneFactory">シーンファクトリ。</param>
    void SetSceneFactory(ISceneFactory* pSceneFactory);

    /// <summary>
    /// シーン遷移エグゼキューターを設定します。
    /// </summary>
    /// <param name="pTransitionExecutor">設定するシーン遷移エグゼキューターへのポインター。</param>
    void SetTransitionExecutor(SceneTransitionExecutor* pTransitionExecutor);

    /// <summary>
    /// 次回シーン生成に渡す引数を設定します。
    /// </summary>
    /// <param name="pSceneArgs">所有権を移動するシーン引数。</param>
    void SetSceneArgs(std::unique_ptr<ISceneArgs> pSceneArgs);

    /// <summary>
    /// モデルマネージャを設定します。
    /// </summary>
    /// <param name="pModelManager">モデルマネージャ。</param>
    void SetModelManager(ModelManager* pModelManager);

    // すべてのシーンに渡す初期引数を追加
    //   - 値はコピーされるため所有権の移動はできない
    //   - キーはユニークであることが保証されるため、同一キー指定時は上書きされる
    /// <summary>
    /// すべてのシーンに渡す初期引数を追加します。
    /// 値はコピーされ、同一キーは上書きされます。
    /// </summary>
    /// <param name="key">引数キー。</param>
    /// <param name="value">引数値。</param>
    /// <returns>自身への参照。</returns>
    SceneManager& AddInitialArg(const std::string& key, const std::any& value);


public:
    /// <summary>
    /// 次フレームで遷移するシーン名を予約します。
    /// </summary>
    /// <param name="sceneName">遷移先シーン名</param>
    void ReserveScene(const std::string& sceneName);
    
    /// <summary>
    /// 次フレームで遷移するシーンを予約します。(遷移付き)
    /// </summary>
    /// <param name="sceneName">遷移先シーン名</param>
    /// <param name="transition">トランジションインスタンス</param>
    void ReserveScene(const std::string& sceneName, std::unique_ptr<TransBase>&& transition);

    /// <summary>
    /// 次フレームで遷移するシーンを予約します。(ロードシーン付き)
    /// </summary>
    /// <param name="sceneName">遷移先シーン名</param>
    /// <param name="loadingName">ロードスクリーン名</param>
    /// <param name="transition">トランジションインスタンス</param>
    void ReserveScene(const std::string& sceneName, const std::string& loadingName, std::unique_ptr<TransBase>&& transition);
    
    /// <summary>
    /// 開始シーンの予約を行います（設定に基づく）。
    /// </summary>
    void ReserveStartupScene();


public: /// シーン動作
    /// <summary>
    /// シーンマネージャを初期化します。
    /// </summary>
    void Initialize();
    
    /// <summary>
    /// シーンの更新を行います（遷移処理を含む）。
    /// </summary>
    void Update();
    
    /// <summary>
    /// シーンの描画（非テキスト）を行います。
    /// </summary>
    void SceneDraw();
    
    /// <summary>
    /// シーン終了処理を行います。
    /// </summary>
    void Finalize();

    /// <summary>
    /// シーンを事前読み込みします。
    /// </summary>
    /// <param name="sceneName">事前読み込みするシーンの名前。</param>
    void ScenePreload(const std::string& sceneName, TaskExecutor& taskExec);

    /// <summary>
    /// デバッグUIを描画します。
    /// </summary>
    void ImGui();


private:
    SceneManager() = default;
    
    /// <summary>
    /// 予約されたシーンへ切り替えます。
    /// </summary>
    void ChangeScene();
    
    /// <summary>
    /// ファクトリーに渡すシーン引数に初期引数をパックします。
    /// </summary>
    void PackSceneArgs();

    std::string name_ = "SceneManager";
    char buffer[128] = {};

    bool isReserveScene_ = false;

    std::unique_ptr<DebugEntry<SceneManager>>   pDebugEntry_        = nullptr;
    std::string                                 nextSceneName_;
    std::unique_ptr<SceneBase>                  pCurrentScene_      = nullptr;
    std::unique_ptr<ISceneArgs>                 pSceneArgs_         = nullptr;
    /// 一時保持用のインスタンス。 ChangeSceneでCurrentScene_に移動する。
    std::unique_ptr<ILoadableScene>             pPreloadedScene_    = nullptr;
    std::unordered_map<std::string, std::any>   initialArgs_;

private: /// 他クラスのインスタンス
    ISceneFactory*              pSceneFactory_          = nullptr;
    SceneTransitionExecutor*    pTransitionExecutor_    = nullptr;
    ModelManager*               pModelManager_          = nullptr;
};