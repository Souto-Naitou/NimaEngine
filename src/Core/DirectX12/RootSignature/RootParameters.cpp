#include "RootParameters.h"
#include <stdexcept>

RootParameters& RootParameters::SetParameter(int index, const std::string& slot, D3D12_SHADER_VISIBILITY visibility)
{
    /// スロット名の妥当性チェック
    if (slot.empty())
        throw std::invalid_argument("Slot name cannot be empty.");

    /// パラメータ配列のサイズを増やす
    if (index >= params_.size()) params_.resize(index + 1);

    /// スロット名の形式チェック
    if (slot.size() < 2 || !std::isdigit(slot[1]))
        throw std::invalid_argument("Slot name format invalid. Expected format like 't0', 'u1', 's2', or 'b0'.");

    /// スロット番号とタイプを解析
    int slotNum = std::stoi(slot.substr(1));
    char typeChar = slot[0];

    D3D12_ROOT_PARAMETER& param = params_[index];
    param.ShaderVisibility = visibility;

    switch (typeChar)
    {
    case 't':
    case 'u':
    case 's':
    {
        auto& range = ranges_.emplace_back();
        range.BaseShaderRegister = slotNum;
        range.NumDescriptors = 1;
        range.RegisterSpace = 0;
        range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        switch (typeChar)
        {
        case 't': range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; break;
        case 'u': range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV; break;
        case 's': range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER; break;
        }

        param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        param.DescriptorTable.NumDescriptorRanges = 1;
        param.DescriptorTable.pDescriptorRanges = &range;
        break;
    }

    case 'b':
        param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        param.Descriptor.ShaderRegister = slotNum;
        param.Descriptor.RegisterSpace = 0;
        break;

    default:
        throw std::invalid_argument("Invalid slot type. Must start with 't', 'u', 's', or 'b'.");
    }

    slotMap_[slot] = index;

    return *this;
}