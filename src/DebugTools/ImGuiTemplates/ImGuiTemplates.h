#pragma once

#ifdef _DEBUG

#include <string>
#include <type_traits>

#include <functional>
#include <Vector2.h>
#include <Vector3.h>
#include <Vector4.h>

#include <imgui.h>
#include <concepts>

/// <summary>
/// ImGuiテンプレート群
/// </summary>
namespace ImGuiTemplate
{
    void VariableTable(const std::string& _stringID, std::function<void()> _function);

    template <typename T>
    void Slider(const std::string& label, T* pValue);

    template <typename T>
    void Drag(const std::string& label, T* pValue);

    template <typename E>
    concept HasEnumCount = std::is_enum_v<E> && requires (E e) { E::COUNT; { ToString(e) }->std::convertible_to<const char*>; };

    template <typename E>
    bool ComboEnum(const char* label, E& v, const std::function<const char*(E)>& pFuncToString, int count);

    template <HasEnumCount E>
    bool ComboEnum(const char* label, E& v);

    /// Implements

    template <typename T>
    void Slider(const std::string& label, T* pValue)
    {
        if constexpr (std::is_same<T, float>::value)
        {
            ImGui::SliderFloat(label.c_str(), pValue, 0.0f, 1.0f);
        }
        else if constexpr (std::is_same<T, int>::value)
        {
            ImGui::SliderInt(label.c_str(), pValue, 0, 100);
        }
        else if constexpr (std::is_same<T, Vector2>::value)
        {
            ImGui::SliderFloat2(label.c_str(), &pValue->x, 0.0f, 1.0f);
        }
        else if constexpr (std::is_same<T, Vector3>::value)
        {
            ImGui::SliderFloat3(label.c_str(), &pValue->x, 0.0f, 1.0f);
        }
        else if constexpr (std::is_same<T, Vector4>::value)
        {
            ImGui::SliderFloat4(label.c_str(), &pValue->x, 0.0f, 1.0f);
        }
        else
        {
            ImGui::Text("ERROR TYPE!");
        }
    }

    template <typename T>
    void Drag(const std::string& label, T* pValue)
    {
        constexpr float kSpeed = 0.01f;

        if constexpr (std::is_same<T, float>::value)
        {
            ImGui::DragFloat(label.c_str(), pValue, kSpeed);
        }
        else if constexpr (std::is_same<T, int>::value)
        {
            ImGui::DragInt(label.c_str(), pValue, 1);
        }
        else if constexpr (std::is_same<T, Vector2>::value)
        {
            ImGui::DragFloat2(label.c_str(), &pValue->x, kSpeed);
        }
        else if constexpr (std::is_same<T, Vector3>::value)
        {
            ImGui::DragFloat3(label.c_str(), &pValue->x, kSpeed);
        }
        else if constexpr (std::is_same<T, Vector4>::value)
        {
            ImGui::DragFloat4(label.c_str(), &pValue->x, kSpeed);
        }
        else
        {
            ImGui::Text("ERROR TYPE!");
        }
    }

    template <typename T>
    void VariableTableRow(const std::string& _varName, const T _var)
    {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        ImGui::Text(_varName.c_str());
        ImGui::TableNextColumn();

        if constexpr (std::is_same<T, bool>::value)
        {
            ImGui::Text("%s", _var ? "true" : "false");
        }
        else if constexpr (std::is_same<T, uint32_t>::value)
        {
            ImGui::Text("%u", _var);
        }
        else if constexpr (std::is_integral<T>::value)
        {
            ImGui::Text("%d", _var);
        }
        else if constexpr (std::is_floating_point<T>::value)
        {
            ImGui::Text("%f", _var);
        }
        else if constexpr (std::is_same<T, std::string>::value)
        {
            ImGui::Text(_var.c_str());
        }
        else if constexpr (std::is_same<T, Vector2>::value)
        {
            ImGui::Text("{ %f, %f }", _var.x, _var.y);
        }
        else if constexpr (std::is_same<T, Vector3>::value)
        {
            ImGui::Text("{ %f, %f, %f }", _var.x, _var.y, _var.z);
        }
        else if constexpr (std::is_same<T, Vector4>::value)
        {
            ImGui::Text("{ %f, %f, %f, %f }", _var.x, _var.y, _var.z, _var.w);
        }
        else
        {
            ImGui::Text("ERROR TYPE!");
        }
    }

    template <typename E>
    bool ComboEnum(const char* label, E& v, const std::function<const char* (E)>& pFuncToString, int count)
    {
        std::vector<const char*> items(count);
        for (int i = 0; i < count; ++i)
        {
            items[i] = pFuncToString(static_cast<E>(i));
        }
        int cur = static_cast<int>(v);
        bool changed = ImGui::Combo(label, &cur, items.data(), count);
        if (changed) v = static_cast<E>(cur);
        return changed;
    }

    template <HasEnumCount E>
    bool ComboEnum(const char* label, E& v)
    {
        return ComboEnum<E>(label, v, [](E e) { return ToString(e); }, static_cast<int>(E::COUNT));
    }
}

#endif // _DEBUG