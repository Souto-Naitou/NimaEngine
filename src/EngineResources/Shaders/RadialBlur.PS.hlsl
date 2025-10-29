#include "Fullscreen.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

struct RadialBlurOption
{
    float2 center;
    int samples;
    float blurWidth; 
};

ConstantBuffer<RadialBlurOption> gOption : register(b0);

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float4 tex = gTexture.Sample(gSampler, input.texcoord);

    float2 direction = input.texcoord - gOption.center;
    float3 outputColor = 0.0f;
    
    for (int sampleIndex = 0; sampleIndex < gOption.samples; ++sampleIndex)
    {
        float2 texcoord = input.texcoord + direction * gOption.blurWidth * float(sampleIndex);
        outputColor += gTexture.Sample(gSampler, texcoord).rgb;
    }

    outputColor *= rcp(float(gOption.samples));

    output.color = float4(outputColor, tex.a);
    return output;
}