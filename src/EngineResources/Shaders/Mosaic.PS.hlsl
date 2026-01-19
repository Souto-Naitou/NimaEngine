#include "FullScreen.hlsli"
#include "RuntimeData.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

cbuffer MosaicOption : register(b1)
{
    float iPower;
    float3 _MosaicOptionPadding0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    float2 id = floor(input.texcoord * iPower);
    input.texcoord = (id + 0.5) / iPower;
    output.color = gTexture.Sample(gSampler, input.texcoord);
    
    return output;
}
