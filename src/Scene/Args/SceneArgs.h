#pragma once

#include <Interfaces/ISceneArgs.h>
#include <unordered_map>

// ISceneArgsの簡易実装クラス
// - コンテナとしてstd::unordered_mapを使用
class SceneArgs : public ISceneArgs
{
public:
    std::any& Get(const std::string& key) override;
    const std::any& Get(const std::string& key) const override;
    void Set(const std::string& key, const std::any& value) override;
    void Reset() override;
    void Erase(const std::string& key) override;

private:
    std::unordered_map<std::string, std::any> argDataMap_;
};