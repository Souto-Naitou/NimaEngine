#pragma once

#include <Core/DirectX12/DirectX12.h>
#include <Features/Viewport/Viewport.h>

#include <wrl.h>
#include <dwrite.h>
#include <map>
#include <unordered_map>
#include <string>
#include <utility>

/// <summary>
/// テキスト描画共通
/// </summary>
class TextSystem : public EngineFeature
{
public:
    TextSystem(const TextSystem&) = delete;
    TextSystem(const TextSystem&&) = delete;
    TextSystem& operator=(const TextSystem&) = delete;
    TextSystem& operator=(const TextSystem&&) = delete;

    static TextSystem* GetInstance()
    {
        static TextSystem instance; return &instance;
    }
    
    void Initialize();
    void PresentDraw();
    void PostDraw();
    void OnResizedWindow();
    void OnResizedBuffers();

public:
    void SetColorBrush(const std::string& key, const D2D1::ColorF& color);
    void SetViewport(Viewport* viewport) { pViewport_ = viewport; }


public: /// Getter
    IDWriteFactory7* GetDWriteFactory() const { return dwriteFactory_.Get(); }
    IDWriteTextFormat* GetTextFormat(const std::string& fontFamily, float fontSize);
    ID2D1SolidColorBrush* GetColorBrush(const std::string& key);
    ID2D1DeviceContext2* GetD2D1DeviceContext() const { return d2dDeviceContext_; }
    Viewport* GetViewport() const { return pViewport_; }


private:
    TextSystem() = default;
    ~TextSystem() = default;

    void CreateDirectWriteFactory();
    void CreateDefaultColorBrush();

private:
    HRESULT hr_ = 0;

    std::map<std::pair<std::string, float>, Microsoft::WRL::ComPtr<IDWriteTextFormat>> textFormats_;
    std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID2D1SolidColorBrush >> colorBrushes_;

    Microsoft::WRL::ComPtr<IDWriteFactory7> dwriteFactory_ = nullptr;      // DirectWrite


private: /// 借り物
    IDXGIDevice* dxgiDevice_ = nullptr;
    ID2D1Factory3* d2dFactory_ = nullptr;
    ID2D1Device2* d2dDevice_ = nullptr;
    ID2D1DeviceContext2* d2dDeviceContext_ = nullptr;
    ID3D11On12Device* d3d11On12Device_ = nullptr;
    ID3D11DeviceContext* d3d11On12DeviceContext_ = nullptr;
    Viewport* pViewport_ = nullptr;
};