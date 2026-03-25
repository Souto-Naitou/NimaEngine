#include "object3d.hlsli"

struct TransformationMatrix
{
    float4x4 WVP;
    float4x4 World;
};

struct Color {
    float4 value;
};

ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);
ConstantBuffer<Color> gColor : register(b1);

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    output.position = mul(input.position, gTransformationMatrix.WVP);
    output.texcoord = input.texcoord;
    output.normal = normalize(mul(input.normal, (float3x3)gTransformationMatrix.World));
    output.worldPosition = mul(input.position, gTransformationMatrix.World).xyz;
    output.color = gColor.value;
    return output;
}