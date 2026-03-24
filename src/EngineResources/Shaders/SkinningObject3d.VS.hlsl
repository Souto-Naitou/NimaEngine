#include "object3d.hlsli"

struct TransformationMatrix
{
    float4x4 WVP;
    float4x4 World;
};

struct Color {
    float4 value;
};

struct Vertex
{
    float4 position;
    float2 texcoord;
    float3 normal;
};

ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);
ConstantBuffer<Color> gColor : register(b1);

struct VertexShaderInput
{
    float4  position : POSITION0;
    float2  texcoord : TEXCOORD0;
    float3  normal : NORMAL0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    Skinned skinned = Skinning(input);
    // Skinning 結果を使って変換
    output.position = mul(skinned.position, gTransformationMatrix.WVP);
    output.worldPosition = mul(skinned.position, gTransformationMatrix.World).xyz;
    output.texcoord = input.texcoord;
    output.normal = normalize(mul(input.normal, (float3x3)gTransformationMatrix.World));
    output.color = gColor.value;
    return output;
}