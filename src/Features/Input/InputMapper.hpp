#pragma once
#include "Input.h"
#include <vector>
#include <variant>
#include <unordered_map>

template <typename Action>
class InputMapper
{
public:
    struct KeyCode
    {
        BYTE value; // DirectInputのキーコード
    };

    struct PadButton
    {
        WORD value; // ゲームパッドのボタン番号
    };

    struct InputActionData
    {
        std::variant<KeyCode, Input::MouseNum, PadButton> inputCode; // キーコード、マウスボタン番号、ゲームパッドボタン番号など
    };

    void MapAction(const Action& action, const InputActionData& actionData)
    {
        actionMap_[action].push_back(actionData);
    }

    bool IsTrigger(const Action& action)
    {
        auto actionDataList = GetInputActionDataList(action);
        if (actionDataList == nullptr) return false;

        /// アクションに関連付けられた入力コードのいずれかがトリガーされた場合にtrueを返す
        for (auto& actionData : *actionDataList)
        {
            bool result = false;

            std::visit([&](auto&& code)
            {
                using T = std::decay_t<decltype(code)>;

                /// 型に応じて適切な入力状態をチェック
                if constexpr (std::is_same_v<T, KeyCode>)
                    result = pInput_->TriggerKey(code.value);
                else if constexpr (std::is_same_v<T, Input::MouseNum>)
                    result = pInput_->TriggerMouse(code);
                else if constexpr (std::is_same_v<T, PadButton>)
                    result = pInput_->TriggerButton(code.value);

            }, actionData.inputCode);

            if (result) return true;
        }
        return false;
    }

    bool IsPush(const Action& action)
    {
        auto actionDataList = GetInputActionDataList(action);
        if (actionDataList == nullptr) return false;

        for (auto& actionData : *actionDataList)
        {
            bool result = false;

            std::visit([&](auto&& code)
            {
                using T = std::decay_t<decltype(code)>;

                if constexpr (std::is_same_v<T, KeyCode>)
                    result = pInput_->PushKey(code.value);
                else if constexpr (std::is_same_v<T, Input::MouseNum>)
                    result = pInput_->PushMouse(code);
                else if constexpr (std::is_same_v<T, PadButton>)
                    result = pInput_->PushButton(code.value);

            }, actionData.inputCode);

            if (result) return true;
        }
        return false;
    }

    bool IsRelease(const Action& action)
    {
        auto actionDataList = GetInputActionDataList(action);
        if (actionDataList == nullptr) return false;

        for (auto& actionData : *actionDataList)
        {
            bool result = false;

            std::visit([&](auto&& code)
            {
                using T = std::decay_t<decltype(code)>;

                if constexpr (std::is_same_v<T, KeyCode>)
                    result = pInput_->ReleaseKey(code.value);
                else if constexpr (std::is_same_v<T, Input::MouseNum>)
                    result = pInput_->ReleaseMouse(code);
                else if constexpr (std::is_same_v<T, PadButton>)
                    result = pInput_->ReleaseButton(code.value);

            }, actionData.inputCode);

            if (result) return true;
        }
        return false;
    }

private:
    const std::vector<InputActionData>* GetInputActionDataList(const Action& action) const
    {
        auto it = actionMap_.find(action);
        if (it != actionMap_.end())
        {
            return &it->second;
        }

        return nullptr; // アクションが見つからない場合
    }

    Input* pInput_ = Input::GetInstance();
    std::unordered_map<Action, std::vector<InputActionData>> actionMap_;
};