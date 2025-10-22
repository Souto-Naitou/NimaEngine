#pragma once

/// <summary>
/// シングルトンパターン
/// </summary>
/// <typeparam name="T">適用するクラス</typeparam>
template <typename T>
class SingletonPattern
{
public:
    SingletonPattern(const SingletonPattern&) = delete;
    SingletonPattern& operator=(const SingletonPattern&) = delete;
    SingletonPattern(SingletonPattern&&) = delete;
    SingletonPattern& operator=(SingletonPattern&&) = delete;

    static T* GetInstance()
    {
        static T instance;
        return &instance;
    }

protected:
    SingletonPattern() = default;
    ~SingletonPattern() = default;
};