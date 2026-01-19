#include "FullScreen.hlsli"
#include "RuntimeData.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

cbuffer ScanlineOption : register(b1)
{   
    float iDivision;
    float iSpeed;
    float iOpacity;
    float iIsOverall;
    float4 iColor0;
    float4 iColor1;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    float pos = input.texcoord.y * iDivision + iTime * iSpeed;
    
    // 0 <= bit < 2.0f
    float bit = fmod(pos, 2.0);
    float dodge = step(1.0, bit);
    float4 lineColor = iColor0 * (1.0f - dodge) + iColor1 * dodge;
    
    float4 texColor = gTexture.Sample(gSampler, input.texcoord);
    output.color = lerp(texColor, lineColor, iOpacity);
    output.color.a = lerp(texColor.a, output.color.a, iIsOverall);
    
    return output;
}