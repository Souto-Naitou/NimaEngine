#pragma once
#include <Features/Layer/Canvas.h>
#include <DebugTools/DebugEntry/DebugEntry.h>
#include <map>
#include <cstdint>
#include <memory>

/// <summary>
/// Z順序付きでキャンバスの呼び出しを行うクラス
/// - ここでいうレイヤーはモノではなく仕組みを指します。
/// </summary>
class OrderedCanvasLayer
{
public:
    using ZOrder = uint32_t;

    OrderedCanvasLayer() = default;
    ~OrderedCanvasLayer() = default;
    
    void Initialize();
    void ImGui();

    // キャンバスの追加 (自動でZオーダーを割り当てる)
    ZOrder AddCanvas(Canvas* canvas);
    // キャンバスの追加 (最前面グループに追加)
    ZOrder AddCanvasTop(Canvas* canvas);

    // キャンバスの削除 (AddCanvasを呼び出した場合、必ず呼ぶ)
    void RemoveCanvas(ZOrder zOrder);
    void RemoveCanvas(Canvas* canvas);

    // フレームの開始処理
    void PreDraw();
    // フレームの終了処理
    void PostDraw();
    // Canvasに登録されているオブジェクトをCanvasに描画する
    void DrawObjects();
    // Canvasにポストエフェクトを適用する
    // 必ず描画命令スレッドとメインスレッドの同期をしてから呼び出すこと
    void ApplyPostEffects();
    // エフェクト適用後のCanvasを描画
    void DrawCanvases();

private:
    constexpr static ZOrder kAutoZBegin_  = 0x00000001;
    constexpr static ZOrder kAutoZEnd_    = 0xFFFFFFFF / 2;
    constexpr static ZOrder kTopZBegin_   = kAutoZEnd_ + 1;

    // デバッグエントリ
    std::unique_ptr<DebugEntry<OrderedCanvasLayer>> pDebugEntry_ = nullptr;
    // <Z-order, Canvas*>
    std::map<ZOrder, Canvas*> canvases_;
    // ポストエフェクト用のコマンドリスト
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> pCommandList_ = nullptr;
    // コマンドアロケータ
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> pCommandAllocator_ = nullptr;
};