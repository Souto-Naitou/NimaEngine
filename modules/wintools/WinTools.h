#pragma once

#include <string>

class WinTools
{
public:
    WinTools(WinTools&) = delete;
    WinTools& operator=(const WinTools&) = delete;
    WinTools(WinTools&&) = delete;
    WinTools& operator=(WinTools&&) = delete;

    static WinTools* GetInstance()
    {
        static WinTools instance;
        return &instance;
    }

    std::string OpenFileDialog(const std::string& defaultPath = {});
    std::string OpenFolderDialog(const std::string& defaultPath = {});

private:
    std::string ShowOpenDialog(const std::string& defaultPath, bool isFolderDialog);

    WinTools();
    ~WinTools();

};