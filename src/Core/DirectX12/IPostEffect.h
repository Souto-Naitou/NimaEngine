#pragma once
#include <d3d12.h>
#include <string>
#include <memory>

class DirectX12;

/// <summary>
/// ポストエフェクト初期化用構造体
/// </summary>
struct PostEffectInitDesc
{
    DirectX12* pDx12 = nullptr;
    ID3D12GraphicsCommandList* pCommandList = nullptr;
};


/// <summary>
/// ポストエフェクトインターフェース
/// </summary>
class IPostEffect
{
public:
    IPostEffect() = default;
    virtual ~IPostEffect() = default;

    /// <summary>
    /// ポストエフェクトを初期化します。
    /// Helper::PostEffect::CreatePostEffect を使用してインスタンスを生成してください。
    /// </summary>
    /// <param name="desc">初期化に必要な DX12 まわりの情報。</param>
    virtual void    Initialize(const PostEffectInitDesc& desc) = 0;
    /// <summary>
    /// リソースを解放します。
    /// </summary>
    virtual void    Finalize() = 0;

    /// <summary>
    /// 有効・無効を設定します。
    /// </summary>
    /// <param name="_flag">true で有効。</param>
    virtual void    Enable(bool _flag) = 0;
    /// <summary>
    /// 有効かどうかを返します。
    /// </summary>
    /// <returns>true なら有効。</returns>
    virtual bool    Enabled() const = 0;

    /// <summary>
    /// 入力テクスチャのハンドルを設定します。
    /// </summary>
    /// <param name="handle">SRV の GPU ディスクリプタハンドル。</param>
    virtual void    SetInputTextureHandle(D3D12_GPU_DESCRIPTOR_HANDLE handle) = 0;
    /// <summary>
    /// 出力テクスチャのハンドルを取得します。
    /// </summary>
    /// <returns>SRV の GPU ディスクリプタハンドル。</returns>
    virtual D3D12_GPU_DESCRIPTOR_HANDLE GetOutputTextureHandle() const = 0;
    /// <summary>
    /// エフェクト名を取得します。
    /// </summary>
    virtual const std::string& GetName() const = 0;

    /// <summary>
    /// 描画のための設定を行います。
    /// </summary>
    virtual void    Setting() = 0;
    /// <summary>
    /// ポストエフェクトを適用します。
    /// </summary>
    virtual void    Apply() = 0;
    /// <summary>
    /// レンダーテクスチャの状態をシェーダーリソース状態へ遷移させます。
    /// </summary>
    virtual void    ToShaderResourceState() = 0;
    /// <summary>
    /// リサイズイベントの事前処理を行います。
    /// </summary>
    virtual void    OnResizeBefore() = 0;
    /// <summary>
    /// バッファリサイズ後の処理を行います。
    /// </summary>
    virtual void    OnResizedBuffers() = 0;
    /// <summary>
    /// デバッグオーバーレイを描画します。
    /// </summary>
    virtual void    DebugOverlay() = 0;
};

