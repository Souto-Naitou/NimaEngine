#pragma once

#include <unordered_map> // unordered_map
#include <typeindex> // type_index
#include <functional>
#include <memory>

/// 依存性注入コンテナ
/// - 生成方法を管理し、依存関係を解決するためのコンテナ
class DIContainer
{
public:
    template <typename T>
    void Register(std::function<std::shared_ptr<T>()> factory)
    {
        factories_[std::type_index(typeid(T))] = factory;
    }

    template <typename T>
    void RegisterSingleton(T* instance)
    {
        Register<T>([instance]() {
            return std::shared_ptr<T>(instance, [](T*) {}); // 第二引数はデリータ(なにもしない)
        });
    }

    template <typename T>
    std::shared_ptr<T> Resolve()
    {
        return std::static_pointer_cast<T>(
            factories_.at(std::type_index(typeid(T)))()
        );
    }


private:
    std::unordered_map < std::type_index, std::function<std::shared_ptr<void>> > factories_;
};