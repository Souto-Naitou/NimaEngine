#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <cstdint>
#include <Vector2.h>
#include <memory>
#include <Features/TimeMeasurer/TimeMeasurer.h>

struct RuntimeDataCommonGPU
{
    Vector2 iResolution;
    float iTime;
    float iDeltaTime;

    uint32_t iFrame;
    float _RuntimeDataPadding0[3];
};

class PostEffectInputCommon
{
public:
    inline static PostEffectInputCommon* GetInstance()
    {
        static PostEffectInputCommon instance;
        return &instance;
    }

    void Initialize(ID3D12Device* pDevice);
    void Update();

    ID3D12Resource* GetBufferResource() const { return pBufferResource_.Get(); }

private:
    void CreateBufferResource();

    ID3D12Device*                           pDevice_            = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource>  pBufferResource_    = nullptr;
    std::unique_ptr<TimeMeasurer>           pTimeMeasurer_      = nullptr;
    RuntimeDataCommonGPU*                   pMappedData_        = nullptr;
};