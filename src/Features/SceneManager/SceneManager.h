#pragma once

#include <Scene/SceneBase.h>
#include <Interfaces/ISceneFactory.h>
#include <Effects/SceneTransition/SceneTransitionExecuter.h>
#include <Features/Model/ModelManager.h>
#include <memory>
#include <Interfaces/ISceneArgs.h>
#include <unordered_map>
#include <DebugTools/ImGuiManager/ImGuiManager.h>
#include <Features/Layer/Layer.h>
#include <Core/DirectX12/DirectX12.h>

/// <summary>
/// シーン保守クラス
/// </summary>
class SceneManager
{
public:
    struct Params
    {
        DirectX12*          pDx12           = nullptr;
        Layer*              pLayer          = nullptr;
        #ifdef _DEBUG
        ImGuiManager*       pImGuiManager   = nullptr;
        #endif // _DEBUG
    };

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
    /// <param name="_pSceneFactory">シーンファクトリ。</param>
    void SetSceneFactory(ISceneFactory* _pSceneFactory);

    /// <summary>
    /// 次回シーン生成に渡す引数を設定します。
    /// </summary>
    /// <param name="_pSceneArgs">所有権を移動するシーン引数。</param>
    void SetSceneArgs(std::unique_ptr<ISceneArgs> _pSceneArgs);

    /// <summary>
    /// モデルマネージャを設定します。
    /// </summary>
    /// <param name="_pModelManager">モデルマネージャ。</param>
    void SetModelManager(ModelManager* _pModelManager);

    // すべてのシーンに渡す初期引数を追加
    //   - 値はコピーされるため所有権の移動はできない
    //   - キーはユニークであることが保証されるため、同一キー指定時は上書きされる
    /// <summary>
    /// すべてのシーンに渡す初期引数を追加します。
    /// 値はコピーされ、同一キーは上書きされます。
    /// </summary>
    /// <param name="_key">引数キー。</param>
    /// <param name="_value">引数値。</param>
    /// <returns>自身への参照。</returns>
    SceneManager& AddInitialArg(const std::string& _key, const std::any& _value);


public:
    /// <summary>
    /// 次フレームで遷移するシーン名を予約します。
    /// </summary>
    /// <param name="_sceneName">遷移先シーン名</param>
    void ReserveScene(const std::string& _sceneName);
    
    /// <summary>
    /// 次フレームで遷移するシーンを予約します。(遷移付き)
    /// </summary>
    /// <param name="_sceneName">遷移先シーン名</param>
    /// <param name="transition">トランジションインスタンス</param>
    void ReserveScene(const std::string& _sceneName, std::unique_ptr<TransBase>&& transition);
    
    /// <summary>
    /// 開始シーンの予約を行います（設定に基づく）。
    /// </summary>
    void ReserveStartupScene();


public: /// シーン動作
    /// <summary>
    /// シーンマネージャを初期化します。
    /// </summary>
    void Initialize(const Params& param);
    
    /// <summary>
    /// シーンの更新を行います（遷移処理を含む）。
    /// </summary>
    void Update();
    
    /// <summary>
    /// シーンの描画（非テキスト）を行います。
    /// </summary>
    void SceneDraw();
    
    /// <summary>
    /// シーンのテキスト描画を行います。
    /// </summary>
    void SceneDrawText();
    
    /// <summary>
    /// シーン終了処理を行います。
    /// </summary>
    void Finalize();


private:
    SceneManager() = default;
    
    /// <summary>
    /// 予約されたシーンへ切り替えます。
    /// </summary>
    void ChangeScene();
    
    /// <summary>
    /// デバッグUIを描画します。
    /// </summary>
    void ImGui();
    
    /// <summary>
    /// ファクトリーに渡すシーン引数に初期引数をパックします。
    /// </summary>
    void PackSceneArgs();

    std::string name_ = "SceneManager";
    char buffer[128] = {};

    bool isReserveScene_ = false;

    Params parameters_ = {};

    std::string nextSceneName_;
    std::unique_ptr<SceneBase> pCurrentScene_ = nullptr;
    std::unique_ptr<ISceneArgs> pSceneArgs_ = nullptr;
    std::unordered_map<std::string, std::any> initialArgs_;

private: /// 他クラスのインスタンス
    ISceneFactory* pSceneFactory_ = nullptr;
    std::unique_ptr<SceneTransitionExecuter> pTransitionExecuter_ = nullptr;

    ModelManager* pModelManager_ = nullptr;
};