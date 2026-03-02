#include "FullScreen.hlsli"
#include "RuntimeData.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

cbuffer ChromaticAberrationOption : register(b1)
{
    float2  iDirection;
    float   iStrength;
    float   _ChromaticAberrationOptionPadding0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    // Normalized pixel coordinates (from 0 to 1)
    float2 uv = input.texcoord;
    float2 reshalf = iResolution.xy / 2.0;

    float2 dir = normalize(iDirection);

    float2 offset = dir * iStrength;
    float r = gTexture.Sample(gSampler, uv + offset).r;
    float g = gTexture.Sample(gSampler, uv).g;
    float b = gTexture.Sample(gSampler, uv - offset).b;

    // Output to screen
    output.color = float4(r, g, b, gTexture.Sample(gSampler, uv).a);
    return output;
}