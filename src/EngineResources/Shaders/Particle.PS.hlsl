#include "Particle.hlsli"

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    float4 textureColor = gTexture.Sample(gSampler, input.texcoord.xy);
    
    // 非プリマルチテクスチャをプリマルチに変換
    textureColor.rgb *= textureColor.a;
    input.color.rgb *= input.color.a;
    
    output.color = textureColor * input.color;

    return output;
}