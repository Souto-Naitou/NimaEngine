#pragma once

#include <drawable/base/DrawableBase.h>
#include <drawable/object3d/Material.h>
#include <drawable/object3d/Object3dInstancedSystem.h>
#include <Features/Model/IModel.h>
#include <Features/Lighting/LightingType.h>
#include <Features/GameEye/GameEye.h>
#include <Features/Lighting/DirectionalLight.h>
#include <Features/Lighting/PointLight.h>
#include <Common/structs.h>

#include <wrl/client.h>
#include <d3d12.h>
#include <vector>
#include <utility>
#include <Vector3.h>
#include <Vector4.h>
#include <Matrix4x4.h>
#include <span>

struct Object3dInstanceData
{
    Vector3     scale       = {};
    Vector3     rotate      = {};
    Vector3     translate   = {};
    Vector4     color       = {};
};

struct Object3dInstanceDataForGPU
{
    Matrix4x4   wvp         = {};
    Matrix4x4   world       = {};
    Vector4     color       = {};
};

/// <summary>
/// インスタンシング描画に対応した3Dオブジェクトクラス
/// </summary>
class Object3dInstanced : public DrawableBase
{
public:
    /// エイリアス
    using value_type = Object3dInstanceData;
    using iterator = std::vector<value_type>::iterator;
    using const_iterator = std::vector<value_type>::const_iterator;
    using reference = value_type&;
    using const_reference = const value_type&;

public:
    struct Option
    {
        MaterialForGPU* pMaterialData       = nullptr;
        LightSetting*   pLightSettingData   = nullptr;
    };

    struct MappedBuffers
    {
        std::span<Object3dInstanceDataForGPU> pInstanceData = {};
        CameraForGPU* pCameraForGPU = nullptr;
    };

    /// ctor / dtor
    Object3dInstanced(uint32_t maxInstanceCount = 1000u);
    ~Object3dInstanced() {};

    /// 初期化処理
    void Initialize();

    /// 更新処理
    void Update();
    void DrawCall(ID3D12GraphicsCommandList* cl) override;

    void SetModel(IModel* pModel)
    { 
        pModel_ = pModel; 
    }

    Option& GetOption() { return option_; }
    const Option& GetOption() const { return option_; }

    #pragma region [ アクセスインターフェース群 ]
    /// [ 追加 ]

    constexpr void push_back(const value_type& instance)
    {
        if (instanceData_.size() < kMaxInstanceCount_)
        {
            instanceData_.push_back(instance);
        }
    }

    template <typename... Args>
    constexpr reference emplace_back(Args&&... args)
    {
        if (instanceData_.size() < kMaxInstanceCount_)
        {
            return instanceData_.emplace_back(std::forward<Args>(args)...);
        }
        assert(false && "instance is full");
        return instanceData_.back();
    }

    /// [ 削除 ]

    constexpr void pop_back()
    {
        if (!empty())
        {
            instanceData_.pop_back();
        }
    }

    constexpr void clear()
    {
        instanceData_.clear();
    }

    /// [ アクセス ]

    constexpr reference operator[](size_t index)
    {
        return instanceData_[index];
    }

    constexpr const_reference operator[](size_t index) const
    {
        return instanceData_[index];
    }

    constexpr size_t size() const noexcept
    {
        return instanceData_.size();
    }

    constexpr bool empty() const noexcept
    {
        return instanceData_.empty();
    }

    /// [ イテレータ ]

    constexpr iterator begin() noexcept
    {
        return instanceData_.begin();
    }

    constexpr iterator end() noexcept
    {
        return instanceData_.end();
    }

    constexpr const_iterator begin() const noexcept
    {
        return instanceData_.begin();
    }

    constexpr const_iterator end() const noexcept
    {
        return instanceData_.end();
    }

    #pragma endregion

private:
    enum class CBufferRegister
    {
        Material            = 0,
        InstanceData        = 1,
        Texture             = 2,
        DirectionalLight    = 3,
        Camera              = 4,
        LightSetting        = 5,
        PointLight          = 6,
        EnvironmentTexture  = 7,
    };

    void CreateInstanceDataResource();
    void CreateCameraForGPUResource();
    void CreateLightSettingResource();
    void CreateMaterialResource();

    const uint32_t          kMaxInstanceCount_  = 0;
    IModel*                 pModel_             = nullptr;
    std::vector<value_type> instanceData_;
    Option                  option_             = {};
    MappedBuffers           mappedBuffers_      = {};

    Microsoft::WRL::ComPtr<ID3D12Resource>  resourceInstanceData_   = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource>  resourceCameraForGPU_   = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource>  resourceLightSetting_   = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource>  resourceMaterial_       = nullptr;

    uint32_t srvIndex_ = 0u;
    D3D12_CPU_DESCRIPTOR_HANDLE srvCpuHandle_ = {};
    D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle_ = {};


private: /// 他クラスが所持するインスタンスへのポインタ
    IGameEye**                  ppGameEye_          = nullptr;
    DirectX12*                  pDx12_              = nullptr;
    ID3D12Device*               pDevice_            = nullptr;
    Object3dInstancedSystem*    pSystem_            = nullptr;
    DirectionalLight*           pDirectionalLight_  = nullptr;
    PointLight*                 pPointLight_        = nullptr;
};