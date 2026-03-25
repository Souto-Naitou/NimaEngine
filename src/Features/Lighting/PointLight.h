#pragma once
#include <memory>

#include <Vector4.h>
#include <Vector3.h>
#include <Features/Model/IModel.h>

class Object3d;

struct alignas(16) PointLightForGPU
{
    Vector4 color           = {}; //!< ライトの色
    Vector3 position        = {}; //!< ライトの位置
    float   intensity       = 0.0f; //!< 輝度
};

/// <summary>
/// ポイントライト
/// </summary>
class PointLight
{
public:
    /// エイリアス
    using value_type = PointLightForGPU;
    using reference = value_type&;
    using const_reference = const value_type&;

public:
    PointLight(ID3D12Device* pDevice) : pDevice_(pDevice) {}

    PointLight(const PointLight&) = delete;
    PointLight& operator=(const PointLight&) = delete;
    PointLight(PointLight&&) noexcept = default;
    PointLight& operator=(PointLight&&) noexcept = default;
    ~PointLight() = default;


    void Initialize();
    void Update();

    inline reference        GetData() { return data_; }
    inline const_reference  GetData() const { return data_; }
    inline ID3D12Resource*  GetResource() const { return pResource_.Get(); }

private:
    /// GPUリソースを作成
    void CreateGPUResource();

    Microsoft::WRL::ComPtr<ID3D12Resource>  pResource_  = nullptr;
    value_type*                             pMapped_    = {};
    value_type                              data_       = {};
    ID3D12Device*                           pDevice_    = nullptr;
};