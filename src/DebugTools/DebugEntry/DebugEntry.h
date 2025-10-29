#pragma once
#include <string>
#include <Utility/Debug/dbgutl.h>
#include <DebugTools/DebugManager/DebugManager.h>

/// <summary>
/// デバッグマネージャー登録補助クラス
/// </summary>
/// <typeparam name="MyType"></typeparam>
template <typename MyType>
class DebugEntry
{
public:
    DebugEntry(const std::string& name, MyType* thisptr, bool isWindow = false);
    DebugEntry(const std::string& category, const std::string& name, MyType* thisptr, bool isWindow = false);
    ~DebugEntry();

    void SetName(const std::string& name) { name_ = utl::debug::generate_name(name, this); }

private:
    std::string category_ = {};
    std::string name_ = "Entry";
};

template<typename MyType>
inline DebugEntry<MyType>::DebugEntry(const std::string& name, MyType* thisptr, bool isWindow)
{
    this->SetName(name);

    DebugManager::GetInstance()->SetComponent(
        name_, 
        std::bind(&MyType::ImGui, thisptr),
        isWindow
    );
}

template<typename MyType>
inline DebugEntry<MyType>::DebugEntry(const std::string& category, const std::string& name, MyType* thisptr, bool isWindow)
{
    this->SetName(name);
    category_ = category;

    DebugManager::GetInstance()->SetComponent(
        category_, 
        name_, 
        std::bind(&MyType::ImGui, thisptr),
        isWindow
    );
}

template<typename MyType>
inline DebugEntry<MyType>::~DebugEntry()
{
    if (category_.empty())
        DebugManager::GetInstance()->DeleteComponent(name_);
    else
        DebugManager::GetInstance()->DeleteComponent(category_, name_);
}
