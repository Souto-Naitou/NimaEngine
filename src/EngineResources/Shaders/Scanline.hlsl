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
    float2 _ScanlineOptionPadding0;
    float4 iColor0;
    float4 iColor1;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    //float  division = 50.0;
    //float  speed = 3.0;
    //float4 color0 = float4(0.7, 0.7, 0.7, 1.0);
    //float4 color1 = float4(1.0, 1.0, 1.0, 1.0);
    
    float pos = input.texcoord.y * iDivision + iTime * iSpeed;
    
    // 0 <= bit < 2.0f
    float bit = fmod(pos, 2.0);
    float dodge = step(1.0, bit);
    output.color = iColor0 * (1.0f - dodge) + iColor1 * dodge;

    return output;
}