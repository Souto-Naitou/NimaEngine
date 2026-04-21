#pragma once
#include <Core/Window/Window.h>

/// <summary>
/// ビューポート単位（vw, vh）を表す型とリテラル演算子群。
/// 注意：シングルビューポートのみ対応
/// </summary>
namespace Math::Viewport::Unit
{
    class vw
    {
    public:
        explicit vw(long double v)
            : value(static_cast<float>(
                static_cast<long double>(Window::clientWidth)* v / 100.0l))
        {
        }

        operator float() const { return value; }

    private:
        float value;
    };

    class vh
    {
    public:
        explicit vh(long double v)
            : value(static_cast<float>
                (static_cast<long double>(Window::clientHeight)* v / 100.0l))
        {
        }

        operator float() const { return value; }

    private:
        float value;
    };
}


inline Math::Viewport::Unit::vw operator"" _vw(long double value)
{
    return Math::Viewport::Unit::vw(value);
}

inline Math::Viewport::Unit::vh operator"" _vh(long double value)
{
    return Math::Viewport::Unit::vh(value);
}

inline Math::Viewport::Unit::vw operator"" _vw(unsigned long long value)
{
    return Math::Viewport::Unit::vw(static_cast<long double>(value));
}

inline Math::Viewport::Unit::vh operator"" _vh(unsigned long long value)
{
    return Math::Viewport::Unit::vh(static_cast<long double>(value));
}