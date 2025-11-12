#include "Fullscreen.hlsli"

static const float PI = 3.14159265f;

Texture2D<float4> gTexture : register(t0);
Texture2D<float> gDepthTexture : register(t1); // 深度テクスチャを使用する場合
SamplerState gSampler : register(s0);

struct DepthOutlineOption
{
    float weightMultiply;
};

struct Material
{
    float4x4 projectionInverse;
};

ConstantBuffer<DepthOutlineOption> gOptions : register(b0); // カーネルのサイズを指定する定数バッファ
ConstantBuffer<Material> gMaterial : register(b1); // マテリアルの定数バッファ

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

float Luminance(float3 color)
{
    return dot(color, float3(0.2125f, 0.7154f, 0.0721f));
}

static const float kPrewittHolizontalKernel[3][3] = {
    { -1.0f / 6.0f, 0.0f, 1.0f / 6.0f },
    { -1.0f / 6.0f, 0.0f, 1.0f / 6.0f },
    { -1.0f / 6.0f, 0.0f, 1.0f / 6.0f }
};

static const float kPrewittVerticalKernel[3][3] = {
    { -1.0f / 6.0f, -1.0f / 6.0f, -1.0f / 6.0f },
    { 0.0f, 0.0f, 0.0f },
    { 1.0f / 6.0f, 1.0f / 6.0f, 1.0f / 6.0f }
};

PixelShaderOutput main(VertexShaderOutput input)
{   
    PixelShaderOutput output;
    
    uint width, height;
    gTexture.GetDimensions(width, height);
    float2 texelSize = float2(rcp(float(width)), rcp(float(height)));
    
    float weight = 0.0f;
    
    float2 difference = float2(0.0f, 0.0f);
    
    for (int i = -1; i <= 1; ++i)
    {
        for (int j = -1; j <= 1; ++j)
        {
            float2 offset = float2(i, j) * texelSize;
            // 現在のtexcoordを算出
            float2 texcoord = input.texcoord + offset;
            // 色に掛けて加算
            float ndcDepth = gDepthTexture.Sample(gSampler, texcoord);
            float4 viewSpace = mul(float4(0.0f, 0.0f, ndcDepth, 1.0f), gMaterial.projectionInverse);
            float viewZ = viewSpace.z * rcp(viewSpace.w);
            difference.x += viewZ * kPrewittHolizontalKernel[i + 1][j + 1];
            difference.y += viewZ * kPrewittVerticalKernel[i + 1][j + 1];
        }
    }
    
    weight = length(difference);
    weight = saturate(weight * gOptions.weightMultiply);
    
    output.color = (1.0f - weight) * gTexture.Sample(gSampler, input.texcoord);
    
    return output;
}