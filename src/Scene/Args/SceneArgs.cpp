#include "SceneArgs.h"

std::any& SceneArgs::Get(const std::string& key)
{
    return argDataMap_.at(key);
}

const std::any& SceneArgs::Get(const std::string& key) const
{
    return argDataMap_.at(key);
}

void SceneArgs::Set(const std::string& key, const std::any& value)
{
    argDataMap_[key] = value;
}

void SceneArgs::Reset()
{
    argDataMap_.clear();
}

void SceneArgs::Erase(const std::string& key)
{
    argDataMap_.erase(key);
}
