#include "Fullscreen.hlsli"

static const float PI = 3.14159265f;

Texture2D<float4> gTexture : register(t0);
Texture2D<float4> gBloomTexture : register(t1);
SamplerState gSampler : register(s0);

struct GaussianBloomOption
{
    float colorMultiply;
};

ConstantBuffer<GaussianBloomOption> gOption : register(b0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{   
    PixelShaderOutput output;
   
    float4 inputColor = gTexture.Sample(gSampler, input.texcoord);
    float4 bloomColor = gBloomTexture.Sample(gSampler, input.texcoord);
    
    output.color = inputColor + bloomColor * gOption.colorMultiply; // 入力の色とブルームの色を加算
    
    return output;
}