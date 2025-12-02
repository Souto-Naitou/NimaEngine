#pragma once

#include <Features/Model/IModel.h>
#include <numbers>
#include <string>
#include <span>
#include <Range.h>

class RingModel : public IModel
{
public:
    struct Params
    {
        DirectX12* pDx12 = nullptr;
        float radiusOuter = 1.0f;
        float radiusInner = 0.2f;
        float radAngleStart = 0.0f;
        float radAngleEnd = 2.0f * std::numbers::pi_v<float>;
        float radAngleOffset = 0.0f;
        std::string textureFilePath;
    };

    RingModel(const Params& params);
    ~RingModel() = default;

    void Initialize() override;
    void Finalize() override;
    void Update() override;
    void Draw(ID3D12GraphicsCommandList* cl) override;
    void ChangeTexture(D3D12_GPU_DESCRIPTOR_HANDLE texSrvHnd) override;

    /// [ リング操作 ]

    /// <summary>
    /// 角度の範囲を設定する。
    /// </summary>
    /// <param name="range">設定する角度の範囲を表す</param>
    void SetAngleRange(const Range<float>& range);

    /// <summary>
    /// 内外半径の範囲を設定する。
    /// </summary>
    /// <param name="range">設定する内外半径の範囲を表す</param>
    void SetRadii(const Range<float>& range);

    /// [ 取得関数 ]
    Range<float> GetAngleRange() const { return Range<float>(params_.radAngleStart, params_.radAngleEnd); }
    Range<float> GetRadii() const { return Range<float>(params_.radiusInner, params_.radiusOuter); }

    /// 内部関数群
    void CreateGPUResource() override;
    bool IsEndLoading() const override;
    void Clone(IModel* src) override;
    std::unique_ptr<IModel> Cloned() override;
    D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const override;
    size_t GetVertexCount() const override;
    D3D12_GPU_DESCRIPTOR_HANDLE GetTextureSrvHandle() const override;
    ModelData* GetModelData() override;

private:
    void CreateVertexResource();
    void LoadTexture();
    void CopyFrom(RingModel* pCopySrc);
    void UpdateVertexData();

    static constexpr uint32_t               kSegmentCount               = 64;  //< 分割数
    Params                                  params_                     = {};
    ModelData                               modelData_                  = {};
    Microsoft::WRL::ComPtr<ID3D12Resource>  vertexResource_             = nullptr;
    D3D12_VERTEX_BUFFER_VIEW                vertexBufferView_           = {};
    std::span<VertexData>                   vertexData_                 = {};
    D3D12_GPU_DESCRIPTOR_HANDLE             textureSrvHandleGPU_        = {};
    bool                                    isReadyDraw_                = false;
    RingModel*                              pCloneSrc_                  = nullptr;  //< クローン元のインスタンス
    bool                                    isOverwroteTexture_         = false;    //< テクスチャを上書きしたかどうか
};