#include "Fullscreen.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

struct RandomFilterOption
{
    float seed;
    float opacity;
};

ConstantBuffer<RandomFilterOption> gOption : register(b0);

float rand2dTo1d(float2 value, float2 dotDir = float2(12.9898, 78.233))
{
    float2 smallValue = sin(value);
    float random = dot(smallValue, dotDir);
    random = frac(sin(random) * 143758.5453);
    return random;
}

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    float random = rand2dTo1d(input.texcoord * (gOption.seed + 0.01));
    
    float4 randColor = float4(random, random, random, 1.0f);

    float4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    
    output.color.rgb = lerp(textureColor.rgb, randColor.rgb, gOption.opacity);
    output.color.a = textureColor.a;
    
    return output;
}