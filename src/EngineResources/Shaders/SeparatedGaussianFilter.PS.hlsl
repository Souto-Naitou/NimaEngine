#include "Fullscreen.hlsli"

static const float PI = 3.14159265f;

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct GaussianFilterOption
{
    int kernelSize;
    float weights[31];
};

struct ExecuteInfo
{
    int2 direction;
};

ConstantBuffer<GaussianFilterOption> gOptions : register(b0); // カーネルのサイズを指定する定数バッファ
ConstantBuffer<ExecuteInfo> gExecuteInfo : register(b1); // カーネルのサイズを指定する定数バッファ

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{   
    PixelShaderOutput output;
    
    uint width, height;
    gTexture.GetDimensions(width, height);
    float2 texelSize = float2(rcp(float(width)), rcp(float(height)));
    
    output.color = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    int kernelSize = gOptions.kernelSize;
    int halfKernelSize = kernelSize * 0.5f;
    
    int isHorizontal = gExecuteInfo.direction.x;
    int isVertical = gExecuteInfo.direction.y;
    
    for (int i = -halfKernelSize; i <= halfKernelSize; ++i)
    {
        float2 offset = float2(i * isHorizontal, i * isVertical) * texelSize;
        output.color += gTexture.Sample(gSampler, input.texcoord + offset) * gOptions.weights[i + halfKernelSize];
    }
    
    return output;
}