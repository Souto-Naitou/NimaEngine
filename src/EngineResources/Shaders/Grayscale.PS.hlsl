#include "Fullscreen.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct GrayscaleOption
{
    float power;
};

ConstantBuffer<GrayscaleOption> gOptions : register(b0); // カーネルのサイズを指定する定数バッファ

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color = gTexture.Sample(gSampler, input.texcoord);
    float value = dot(output.color.rgb, float3(0.2125f, 0.7154f, 0.0721f));
    output.color.rgb = lerp(output.color.rgb, float3(value, value, value), gOptions.power);
    return output;
}