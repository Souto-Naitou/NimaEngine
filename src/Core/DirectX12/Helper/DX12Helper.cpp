#include "DX12Helper.h"

#pragma warning(disable: 6387) // 例外の可能性がある関数の警告を無視

#include <cassert>
#include <DebugTools/Logger/Logger.h>
#include <format>
#include <Utility/ConvertString/ConvertString.h>
#include <Core/DirectX12/DirectX12.h>
#include <Features/Model/ModelData.h>
#include <Core/DirectX12/SRVManager.h>
#include <d3dx12.h>
#include <config/EngineSetting.h>
#include <Core/Window/Window.h>

using namespace DX12Helper;

void DX12Helper::CreateDevice(ComPtr<ID3D12Device>& device, ComPtr<IDXGIAdapter4>& adapter)
{
    /// D3D12Deviceの生成

    // 機能レベルとログ出力用の文字列
    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0
    };
    const char* featureLevelStrings[] = { "12.2", "12.1", "12.0" };
    // 高い順に生成できるか試していく
    for (size_t i = 0; i < _countof(featureLevels); ++i)
    {
        // 採用したアダプターでデバイスを生成
        HRESULT hr = D3D12CreateDevice(adapter.Get(), featureLevels[i], IID_PPV_ARGS(device.GetAddressOf()));
        // 指定した機能レベルでデバイスが生成できたかを確認
        if (SUCCEEDED(hr))
        {
            // 生成できたのでログ出力を行ってループを抜ける
            Logger::GetInstance()->LogInfo(__FILE__, __FUNCTION__, std::format("FeatureLevel : {}", featureLevelStrings[i]));
            break;
        }
    }
    // デバイスの生成がうまくいかなかったので起動できない
    if (!device)
    {
        Logger::GetInstance()->LogError(__FILE__, __FUNCTION__, "Failed to create");
        assert(device && "Failed to create device");
    }

    Logger::GetInstance()->LogInfo(__FILE__, __FUNCTION__, "Initilization succeeded"); // 初期化完了のログを出力
}


//#ifdef _DEBUG
void DX12Helper::PauseError(ComPtr<ID3D12Device>& device, ComPtr<ID3D12InfoQueue>& infoQ)
{

    if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(infoQ.GetAddressOf()))))
    {
        // やばいエラー時に止まる
        infoQ->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
        // エラー時に止まる <- 解放忘れが判明したら、コメントアウト
        infoQ->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
        //警告時に止まる
        infoQ->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

        // 抑制するメッセージのID
        D3D12_MESSAGE_ID denyIds[] = {
            // Windows11でのDXGIデバッグレイヤーとDX12デバッグレイヤーの相互作用バグによるエラーメッセージ
            D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE,
            D3D12_MESSAGE_ID_CREATERESOURCE_STATE_IGNORED
        };

        // 抑制するレベル
        D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
        D3D12_INFO_QUEUE_FILTER filter{};
        filter.DenyList.NumIDs        = _countof(denyIds);
        filter.DenyList.pIDList       = denyIds;
        filter.DenyList.NumSeverities = _countof(severities);
        filter.DenyList.pSeverityList = severities;
        // 指定したメッセージの表示を制限する
        infoQ->PushStorageFilter(&filter);
    }

}
//#endif // _DEBUG

ComPtr<ID3D12Resource> DX12Helper::CreateDepthStencilTextureResource(const ComPtr<ID3D12Device>& device, int32_t width, int32_t height)
{
    D3D12_RESOURCE_DESC resourceDesc{};
    resourceDesc.Width            = width;                                     // 幅
    resourceDesc.Height           = height;                                    // 高さ
    resourceDesc.MipLevels        = 1;                                          // mipmapの数
    resourceDesc.DepthOrArraySize = 1;	                                        // 奥行き or 配列Textureの配列数
    resourceDesc.Format           = DXGI_FORMAT_D24_UNORM_S8_UINT;              // フォーマット
    resourceDesc.SampleDesc.Count = 1;                                          // サンプリング数
    resourceDesc.Dimension        = D3D12_RESOURCE_DIMENSION_TEXTURE2D;         // 2DTexture
    resourceDesc.Flags            = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;    // DepthStencilとして使う通知


    /// 利用するHeapの設定
    D3D12_HEAP_PROPERTIES heapProperties{};
    heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;                  // VRAMに


    /// 深度値のクリア設定
    D3D12_CLEAR_VALUE depthClearValue{};
    depthClearValue.DepthStencil.Depth = 1.0f;                          // 1.0f(最大値)でクリア
    depthClearValue.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT; // フォーマット。Resourceと合わせる


    /// Resourceの生成
    ComPtr<ID3D12Resource> resource = nullptr;
    HRESULT hr = device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &depthClearValue,
        IID_PPV_ARGS(&resource)
    );

    if (FAILED(hr))
    {
        Logger::GetInstance()->LogError(__FILE__, __FUNCTION__, "Failed to Create DSTR");
        assert(false && "Failed create depth stencil texture resource");
    }


    return resource;
}

ComPtr<IDxcBlob> DX12Helper::CompileShader(
    const std::wstring& filePath,
    const wchar_t* profile,
    const ComPtr<IDxcUtils>& dxcUtils,
    const ComPtr<IDxcCompiler3>& dxcCompiler,
    const ComPtr<IDxcIncludeHandler>& includeHandler
)
{
    /// 1. hlslファイルを読み込む

    // これからシェーダーをコンパイルする旨をログに出す
    std::string filepathA = ConvertString(filePath);
    std::string profileA = ConvertString(profile);
    Logger::GetInstance()->LogInfo(
        __FILE__,
        __FUNCTION__,
        std::format("Begin compile. Path:{}, Profile:{}", filepathA, profileA)
    );

    // hlslファイルを読む
    ComPtr<IDxcBlobEncoding> shaderSource = nullptr;
    HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);

    // 読めなかったら止める
    if (FAILED(hr))
    {
        Logger::GetInstance()->LogError(__FILE__, __FUNCTION__, "Failed load file");

        assert(false && "Failed loadfile.");
    }

    // 読み込んだファイルの内容を設定する
    DxcBuffer shaderSourceBuffer;
    shaderSourceBuffer.Ptr      = shaderSource->GetBufferPointer();
    shaderSourceBuffer.Size     = shaderSource->GetBufferSize();
    shaderSourceBuffer.Encoding = DXC_CP_UTF8; // UTF-8の文字コードであることを通知

    /// 2. Compileする
    #ifdef _DEBUG
    LPCWSTR arguments[] = {
        filePath.c_str(),           // コンパイル対象のhlslファイル名
        L"-E", L"main",             // エントリーポイントの指定。基本的にmain以外にはしない
        L"-T", profile,             // ShaderProfileの設定
        L"-Zi", L"-Qembed_debug",   // デバッグ用の情報を埋め込む
        L"-Od",                     // 最適化を外しておく
        L"-Zpr",                    // メモリレイアウトは行優先
    };
    #else
    LPCWSTR arguments[] = {
        filePath.c_str(),           // コンパイル対象のhlslファイル名
        L"-E", L"main",             // エントリーポイントの指定。基本的にmain以外にはしない
        L"-T", profile,             // ShaderProfileの設定
        L"-Zi", L"-Qembed_debug",   // デバッグ用の情報を埋め込む
        L"-O3",                     // 最適化を有効に
        L"-Zpr",                    // メモリレイアウトは行優先
    };
    #endif

    // 実際にShaderをコンパイルする
    ComPtr<IDxcResult> shaderResult = nullptr;
    hr = dxcCompiler->Compile(
        &shaderSourceBuffer,        // 読み込んだファイル
        arguments,                  // コンパイルオプション
        _countof(arguments),        // コンパイルオプションの数
        includeHandler.Get(),       // includeが含まれた諸々
        IID_PPV_ARGS(&shaderResult) // コンパイル結果
    );

    // コンパイルエラーではなくdxcが起動できないなど致命的な状況
    if (FAILED(hr))
    {
        Logger::GetInstance()->LogError(__FILE__, __FUNCTION__, "Failed launch DXC compiler");
        assert(false && "Failed compile shader");
    }

    /// 3. 警告・エラーが出ていないか確認する
    ComPtr<IDxcBlobUtf8> shaderError = nullptr;
    shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
    if (shaderError != nullptr && shaderError->GetStringLength() != 0)
    {
        Logger::GetInstance()->LogError(__FILE__, __FUNCTION__, shaderError->GetStringPointer());
        assert(false);
    }

    /// 4. Compile結果を受け取って返す

    // コンパイル結果から実行用のバイナリ部分を取得
    ComPtr<IDxcBlob> shaderBlob = nullptr;
    hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
    assert(SUCCEEDED(hr));

    // 成功したログを出す
    Logger::GetInstance()->LogInfo(
        __FILE__,
        __FUNCTION__,
        "Compile succeeded." + std::format(" Size: {}bytes", shaderBlob->GetBufferSize())
    );

    // 実行用のバイナリを返却
    return shaderBlob;
}

ComPtr<ID3D12Resource> DX12Helper::CreateBufferResource(const ComPtr<ID3D12Device>& _device, size_t _sizeInBytes)
{
    return CreateBufferResource(_device, _sizeInBytes, D3D12_RESOURCE_FLAG_NONE);
}

ComPtr<ID3D12Resource> DX12Helper::CreateBufferResource(const ComPtr<ID3D12Device>& _device, size_t _sizeInBytes, D3D12_RESOURCE_FLAGS _flag)
{
    ComPtr<ID3D12Resource> result = nullptr;
    D3D12_HEAP_PROPERTIES uploadHeapProperties{};
    // UPLOADヒープのリソースはGENERIC_READで生成する必要がある
    D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON;
    if (_flag & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)
    {
        uploadHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
    }
    else
    {
        uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
        uploadHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        initialState = D3D12_RESOURCE_STATE_GENERIC_READ;
    }

    D3D12_RESOURCE_DESC desc{};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Width = _sizeInBytes;
    desc.Height = 1;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.SampleDesc.Count = 1;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags = _flag;
    HRESULT hr = _device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
        &desc, initialState, nullptr,
        IID_PPV_ARGS(&result));

    if (FAILED(hr))
    {
        Logger::GetInstance()->LogError(__FILE__, __FUNCTION__, "CreateCommittedResource failed");
        assert(false && "Create committed resource failed");
    }

    return result;
}

ComPtr<ID3D12Resource> DX12Helper::CreateTextureResource(const ComPtr<ID3D12Device>& _device, const DirectX::TexMetadata& _metadata)
{
    bool isCubeMap = _metadata.IsCubemap();

    if (isCubeMap)
    {
        // メタデータをログ出力
        Logger::GetInstance()->LogInfo(
            __FILE__,
            __FUNCTION__,
            std::format(
                "Format={}, Width={}, Height={}, Mip={}, Array={}, Cube={}",
                static_cast<int>(_metadata.format), static_cast<int>(_metadata.width), static_cast<int>(_metadata.height),
                static_cast<int>(_metadata.mipLevels), static_cast<int>(_metadata.arraySize), isCubeMap));
    }

    // metadataをもとにResourceの設定
    D3D12_RESOURCE_DESC resourceDesc{};
    resourceDesc.Width              = UINT(_metadata.width);
    resourceDesc.Height             = UINT(_metadata.height);
    resourceDesc.MipLevels          = UINT16(_metadata.mipLevels);
    resourceDesc.DepthOrArraySize   = isCubeMap ? 6 : UINT16(_metadata.arraySize);
    resourceDesc.Format             = _metadata.format;
    resourceDesc.SampleDesc.Count   = 1;
    resourceDesc.Dimension          = D3D12_RESOURCE_DIMENSION(_metadata.dimension);


    // Resourceの生成
    ComPtr<ID3D12Resource> resource = nullptr;
    D3D12_HEAP_PROPERTIES heapProperties{};
    heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // 利用するHeapの設定
    HRESULT hr = _device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&resource));
    if (FAILED(hr)) // 生成に失敗したら止める
    {
        Logger::GetInstance()->LogError(__FILE__, __FUNCTION__, "CreateCommittedResource failed");
        assert(false && "Create committed resource failed");
    }

    return resource;
}

DX12Resource DX12Helper::CreateDX12ResourceForRender(
    const ComPtr<ID3D12Device>& device,
    RTVHeapCounter* rtvHeapCounter,
    const std::string& name)
{
    DX12Resource result;

    const auto kFormat = NimaEngine::Config::kRenderTargetFormat;

    /// リソースの作成と初期化
    {
        auto temp = DX12Helper::CreateResourceForRenderTarget(
            device,
            Window::clientWidth,
            Window::clientHeight,
            kFormat,
            NimaEngine::Config::kEditorBGColor
        );

        DX12Resource::Params param{};
        param.resource = temp;
        param.state = D3D12_RESOURCE_STATE_RENDER_TARGET;
        param.format = kFormat;
        param.name = name;

        result.Initialize(param);
    }

    /// RTVの作成
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
    rtvDesc.Format = kFormat;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

    if (result.GetRTVHandle().ptr == 0)
    {
        const auto rtvHandleIndex = rtvHeapCounter->Allocate();
        const auto rtvCPUHandle = rtvHeapCounter->GetRTVHandle(rtvHandleIndex);
        result.SetRTV(rtvHandleIndex, rtvCPUHandle);
    }

    device->CreateRenderTargetView(
        result.GetResource().Get(),
        &rtvDesc,
        result.GetRTVHandle()
    );

    return result;
}

DX12Helper::ComPtr<ID3D12Resource> DX12Helper::UploadTextureData(
    DX12Resource& texture,
    const DirectX::ScratchImage& mipImages,
    const ComPtr<ID3D12Device>& device,
    const ComPtr<ID3D12GraphicsCommandList>& commandList
)
{
    std::vector<D3D12_SUBRESOURCE_DATA> subresources;
    DirectX::PrepareUpload(device.Get(),
        mipImages.GetImages(), 
        mipImages.GetImageCount(), 
        mipImages.GetMetadata(), 
        subresources
    );

    uint64_t sizeIntermediate = GetRequiredIntermediateSize(
        texture.GetResource().Get(),
        0,
        static_cast<UINT>(subresources.size())
    );

    ComPtr<ID3D12Resource> resourceIntermediate = DX12Helper::CreateBufferResource(
        device, 
        sizeIntermediate
    );

    UpdateSubresources(
        commandList.Get(),
        texture.GetResource().Get(),
        resourceIntermediate.Get(),
        0,
        0,
        static_cast<UINT>(subresources.size()), // NumSubresources
        subresources.data()
    );

    // Resourceの状態を更新
    texture.GetStateTracker().ChangeState(commandList.Get(), D3D12_RESOURCE_STATE_GENERIC_READ);

    return resourceIntermediate;
}

ComPtr<ID3D12Resource> DX12Helper::CreateVertexResource(const ComPtr<ID3D12Device>& _device, unsigned int _countVertex)
{
    // 頂点リソースを作る
    ComPtr<ID3D12Resource> vertexResource = CreateBufferResource(_device, sizeof(VertexData) * _countVertex);
    return vertexResource;
}

void DX12Helper::ChangeStateResource(const ComPtr<ID3D12GraphicsCommandList>& _commandList, const ComPtr<ID3D12Resource>& _resource, D3D12_RESOURCE_STATES _before, D3D12_RESOURCE_STATES _after)
{
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = _resource.Get();
    barrier.Transition.StateBefore = _before;
    barrier.Transition.StateAfter = _after;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    _commandList->ResourceBarrier(1, &barrier);
}

void DX12Helper::CommandListCommonSetting(const DirectX12* _pDx12, ID3D12GraphicsCommandList* _commandList, const D3D12_CPU_DESCRIPTOR_HANDLE* rtvHandle)
{
    /// 必要なデータを取得
    const SRVManager* pSrvManager = SRVManager::GetInstance();
    ID3D12DescriptorHeap* ppHeaps[] = { pSrvManager->GetDescriptorHeap() };

    auto dsvHandle      = _pDx12->GetDSVDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
    auto& viewport       = _pDx12->GetViewport();
    auto& scissorRect    = _pDx12->GetScissorRect();

    /// コマンドリストの設定
    _commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
    if (rtvHandle)
    {
        _commandList->OMSetRenderTargets(1, rtvHandle, FALSE, &dsvHandle);
    }
    _commandList->RSSetViewports(1, &viewport);
    _commandList->RSSetScissorRects(1, &scissorRect);

    return;
}

ComPtr<ID3D12Resource> DX12Helper::CreateResourceForRenderTarget(const ComPtr<ID3D12Device>& _device, int32_t _width, int32_t _height, DXGI_FORMAT _format, const Vector4& _clearColor)
{
    ComPtr<ID3D12Resource> result = nullptr;
    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resourceDesc.Width = _width;
    resourceDesc.Height = _height;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    resourceDesc.Format = _format;

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = _format;
    clearValue.Color[0] = _clearColor.x;
    clearValue.Color[1] = _clearColor.y;
    clearValue.Color[2] = _clearColor.z;
    clearValue.Color[3] = _clearColor.w;

    HRESULT hr = _device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        &clearValue,
        IID_PPV_ARGS(&result));

    if (FAILED(hr))
    {
        Logger::GetInstance()->LogError(__FILE__, __FUNCTION__, "CreateCommittedResource failed");
        assert(false && "Create committed resource failed");
    }

    return result;
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12Helper::GetCPUDescriptorHandle(const ComPtr<ID3D12DescriptorHeap>& _descriptorHeap, uint32_t _descriptorSize, uint32_t _index)
{
    D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = _descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    handleCPU.ptr += (_descriptorSize * _index); // ポインタをヒープの始めからインデックス分インクリメント
    return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE DX12Helper::GetGPUDescriptorHandle(const ComPtr<ID3D12DescriptorHeap>& _descriptorHeap, uint32_t _descriptorSize, uint32_t _index)
{
    D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = _descriptorHeap->GetGPUDescriptorHandleForHeapStart();
    handleGPU.ptr += (_descriptorSize * _index);
    return handleGPU;
}