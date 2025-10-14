#pragma once

#include <Core/DirectX12/DX12Resource/DX12Resource.h>
#include <string>

class Canvas
{
public:
    Canvas();
    ~Canvas();

    // Not copyable
    Canvas(const Canvas&) = delete;
    Canvas& operator=(const Canvas&) = delete;
    // Not movable
    Canvas(Canvas&&) = delete;
    Canvas& operator=(Canvas&&) = delete;

    // Setters
    inline void SetEnable(bool _enable) { isEnabled_ = _enable; }
    void SetName(const std::string& _name);

    // Getters
    [[nodiscard]]
    inline bool IsEnabled() const { return isEnabled_; }
    [[nodiscard]]
    inline const std::string& GetName() const { return name_; }
    [[nodiscard]]
    inline const DX12Resource& GetResource() const { return resource_; }

    void Initialize();
    void Draw() const;

private:
    DX12Resource resource_;
    std::string name_ = "Canvas";
    bool isEnabled_ = false;
};