#pragma once
#include <Vector4.h>
#include <Vector3.h>
#include <wrl/client.h>
#include <d3d12.h>
#include <span>


struct DirectionalLightForGPU
{
    Vector4 color       = {}; //!< ライトの色
    Vector3 direction   = {}; //!< ライトの向き (正規化必須)
    float   intensity   = {}; //!< 輝度
};

class DirectionalLight
{
public:
    /// エイリアス
    using value_type = DirectionalLightForGPU;
    using reference = value_type&;
    using const_reference = const value_type&;

public:
    DirectionalLight(ID3D12Device* pDevice) : pDevice_(pDevice) {}

    DirectionalLight(const DirectionalLight&) = delete;
    DirectionalLight& operator=(const DirectionalLight&) = delete;
    DirectionalLight(DirectionalLight&&) noexcept = default;
    DirectionalLight& operator=(DirectionalLight&&) noexcept = default;
    ~DirectionalLight() = default;

    void Initialize();
    void Update();

    inline reference        GetData() { return data_; }
    inline const_reference  GetData() const { return data_; }
    inline ID3D12Resource*  GetResource() const { return pResource_.Get(); }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource>  pResource_  = nullptr;
    value_type*                             pMapped_    = {};
    value_type                              data_       = {};
    ID3D12Device*   pDevice_ = nullptr;
};