#include "Line.hlsli"

struct VertexShaderInput
{
    float3 position : POSITION0;
    
};

struct Matrices
{
    float4x4 WVP;
};

ConstantBuffer<Matrices> gWVPMatrix : register(b0);

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    float4 pos = mul(float4(input.position, 1.0f), gWVPMatrix.WVP);

    output.position = pos;
    return output;
}