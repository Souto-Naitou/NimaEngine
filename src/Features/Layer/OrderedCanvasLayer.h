#pragma once
#include <list>
#include <Features/Layer/Canvas.h>

class OrderedCanvasLayer
{
public:
    OrderedCanvasLayer() = default;
    ~OrderedCanvasLayer() = default;
    
    void Initialize();

    uint32_t AddCanvas(Canvas* canvas, uint32_t zOrder = 0);

    void RemoveCanvas(uint32_t zOrder);
    void RemoveCanvas(Canvas* canvas);

    // フレームの開始処理
    void PreDraw();

    void PostDraw();

    // Canvasに登録されているオブジェクトをCanvasに描画する
    void DrawObjects();

    // Canvasにポストエフェクトを適用する
    // 必ず描画命令スレッドとメインスレッドの同期をしてから呼び出すこと
    void ApplyPostEffects();
    
    // エフェクト適用後のCanvasを描画
    void DrawCanvases();

private:
    // <Z-order, Canvas*>
    std::map<uint32_t, Canvas*> canvases_;
    uint32_t nextZOrder_ = 1;

    // ポストエフェクト用のコマンドリスト
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> pCommandList_ = nullptr;
    // コマンドアロケータ
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> pCommandAllocator_ = nullptr;
};