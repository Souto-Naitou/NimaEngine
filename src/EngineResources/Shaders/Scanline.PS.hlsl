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

    // 0,0f <= bit < 1.0f なら dodge = 0.0f
    // 1.0f <= bit < 2.0f なら dodge = 1.0f
    float dodge = step(1.0, bit);
    
    float4 lineColor = lerp(iColor0, iColor1, dodge);
    lineColor.a *= iOpacity;
    
    float4 texColor = gTexture.Sample(gSampler, input.texcoord);
    
    bool isTransparent = texColor.a < 0.01f;
    output.color = lerp(texColor, lineColor, lineColor.a);
    output.color = lerp(output.color, texColor, isTransparent && !iIsOverall);

    return output;
}