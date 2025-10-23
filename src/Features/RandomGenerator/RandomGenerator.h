#pragma once

#include <random>

/// <summary>
/// 乱数生成補助クラス
/// </summary>
class RandomGenerator
{
public:
    RandomGenerator(const RandomGenerator&) = delete;
    RandomGenerator& operator=(const RandomGenerator&) = delete;
    RandomGenerator(RandomGenerator&&) = delete;
    RandomGenerator& operator=(RandomGenerator&&) = delete;

    static RandomGenerator* GetInstance()
    {
        static RandomGenerator instance;
        return &instance;
    }

    /// <summary>
    /// 乱数エンジンを初期化します。
    /// </summary>
    void Initialize();
    /// <summary>
    /// 終了処理を行います。
    /// </summary>
    void Finalize();

    template <class T>
    /// <summary>
    /// 指定範囲 [min, max] の一様分布から実数乱数を生成します。
    /// </summary>
    /// <param name="_min">下限。</param>
    /// <param name="_max">上限。</param>
    /// <returns>生成された乱数。</returns>
    T Generate(T _min, T _max)
    {
        T trueMin = _min < _max ? _min : _max;
        T trueMax = _min < _max ? _max : _min;
        std::uniform_real_distribution<T> dist(trueMin, trueMax);
        return dist(engine_);
    }

private:
    RandomGenerator() = default;
    ~RandomGenerator() = default;

private:
    std::random_device seed_;
    std::mt19937 engine_;
};