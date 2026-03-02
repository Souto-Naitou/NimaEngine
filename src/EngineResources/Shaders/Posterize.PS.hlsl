#include "Fullscreen.hlsli"
#include "RuntimeData.hlsli"

Texture2D gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

cbuffer PosterizeOption : register(b1)
{
    float   iLevel;
    float3  _PosterizeOptionPadding0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
        
    float2 uv = input.texcoord;
    
    float4 color = gTexture.Sample(gSampler, uv);
    output.color.a = color.a;
    color = floor(color * iLevel) / iLevel;
   
    // Output to screen
    output.color.rgb = color.rgb;
    return output;
}