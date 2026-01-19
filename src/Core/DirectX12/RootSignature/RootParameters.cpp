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

    auto& param = params_[index];
    param.visibility = visibility;

    switch (typeChar)
    {
    case 't':
    case 'u':
    case 's':
    {
        auto offset = static_cast<uint32_t>(ranges_.size());
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

        param.type = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        param.table.rangeCount = static_cast<uint32_t>(ranges_.size()) - offset;
        param.table.rangeOffset = offset;
        break;
    }

    case 'b':
        param.type = D3D12_ROOT_PARAMETER_TYPE_CBV;
        param.descriptor.ShaderRegister = slotNum;
        param.descriptor.RegisterSpace = 0;
        break;

    default:
        throw std::invalid_argument("Invalid slot type. Must start with 't', 'u', 's', or 'b'.");
    }

    slotMap_[slot].push_back(index);

    return *this;
}

const D3D12_ROOT_PARAMETER* RootParameters::BuildParams()
{
    d3dParams_.clear();
    d3dParams_.reserve(params_.size());

    for (const auto& param : params_)
    {
        auto& d3dParam = d3dParams_.emplace_back();
        d3dParam.ParameterType = param.type;
        d3dParam.ShaderVisibility = param.visibility;

        switch (param.type)
        {
        case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
            d3dParam.DescriptorTable.NumDescriptorRanges = param.table.rangeCount;
            d3dParam.DescriptorTable.pDescriptorRanges = &ranges_[param.table.rangeOffset];
            break;
        case D3D12_ROOT_PARAMETER_TYPE_CBV:
            d3dParam.Descriptor.ShaderRegister = param.descriptor.ShaderRegister;
            d3dParam.Descriptor.RegisterSpace = param.descriptor.RegisterSpace;
            break;
        default:
            throw std::runtime_error("Unsupported root parameter type.");
        }
    }

    return d3dParams_.data();
}
