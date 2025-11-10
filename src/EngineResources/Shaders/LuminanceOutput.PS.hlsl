#include "Fullscreen.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct LuminanceOutputOption
{
    float threshold;
};

ConstantBuffer<LuminanceOutputOption> gOptions : register(b0); // カーネルのサイズを指定する定数バッファ

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

float luminance(float3 color)
{
    return dot(color, float3(0.2126, 0.7152, 0.0722)); // Rec.709
}

PixelShaderOutput main(VertexShaderOutput input)
{   
    PixelShaderOutput output;
    
    float4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    
    float bright = luminance(textureColor.rgb);
    
    float mask = step(gOptions.threshold, bright); // lum >= threshold → 1.0, それ以外 → 0.0
    output.color = textureColor * mask;
    return output;
}