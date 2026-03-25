#include "object3d.hlsli"

struct Camera
{
    float3 worldPosition;
};

struct Material
{
    float4x4 uvTransform;
    float shininess;
    float environmentCoefficient;
    float2 tilingMultiply; //!< UVのタイリング係数
};

struct Lighting
{
    int enableDirectionalLight;
    int enablePointLight;
    int lightingType;
};

struct PointLight
{
    float4  color;              //!< ライトの色
    float3  position;           //!< ライトの位置
    float   intensity;          //!< 輝度
};

struct DirectionalLight
{
    float4 color; //!< ライトの色
    float3 direction; //!< ライトの向き(正規化必須)
    float intensity; //!< 輝度
};

ConstantBuffer<Material>            gMaterial           : register(b0);
ConstantBuffer<DirectionalLight>    gDirectionalLight   : register(b1);
ConstantBuffer<Camera>              gCamera             : register(b3);
ConstantBuffer<Lighting>            gLighting           : register(b4);
ConstantBuffer<PointLight>          gPointLight         : register(b5);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

Texture2D<float4>   gTexture            : register(t0);
TextureCube<float4> gEnvironmentTexture : register(t1);
SamplerState        gSampler            : register(s0);

[earlydepthstencil]
PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color = float4(0.0f, 0.0f, 0.0f, 0.0f);

    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);

    transformedUV.xy *= gMaterial.tilingMultiply;

    float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    
    float3 pointLightDir = normalize(gPointLight.position - input.worldPosition);

    float3 cameraToPosition = normalize(input.worldPosition - gCamera.worldPosition);
    float3 reflectedVector = reflect(cameraToPosition, normalize(input.normal));
    float4 environmentColor = gEnvironmentTexture.Sample(gSampler, reflectedVector);
    
    if (gLighting.enableDirectionalLight != 0)
    {
        float cos;
        if (gLighting.lightingType == 0)
        {
            cos = saturate(dot(input.normal, -gDirectionalLight.direction));
        }
        else if (gLighting.lightingType == 1)
        {
            // half lambert
            float NdotL = dot(normalize(input.normal), -gDirectionalLight.direction);
            cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
        }

        float3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
        float3 halfVector = normalize(-gDirectionalLight.direction + toEye);
        float NdotH = dot(normalize(input.normal), halfVector);
        float specularPow = pow(saturate(NdotH), gMaterial.shininess); // 反射強度
        
        float3 diffuse = input.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
        float3 specular = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * float3(1.0f, 1.0f, 1.0f);

        /// ポイントライト
        if (gLighting.enablePointLight == 1)
        {
            float distance = length(gPointLight.position - input.worldPosition); // ポイントライトへの距離
            if (distance < 0.1f) distance = 0.1f; // 0除算対策
            float factor = 1.0f / (distance * distance); // 距離の2乗に反比例する
            
            float3 pointLightDiffuse = input.color.rgb * textureColor.rgb * gPointLight.color.rgb * saturate(dot(input.normal, pointLightDir)) * gPointLight.intensity * factor;
            float3 pointLightSpecular = gPointLight.color.rgb * gPointLight.intensity * specularPow * float3(1.0f, 1.0f, 1.0f) * factor;
            
            output.color.rgb = diffuse + specular + pointLightDiffuse + pointLightSpecular;
        }
        else
        {
            output.color.rgb = diffuse + specular;
        }
        output.color.a = input.color.a * textureColor.a;
    }
    else
    {
        output.color = input.color * textureColor;
    }

    //if (textureColor.a <= 0.f || output.color.a <= 0.f)
    //{
    //    discard;
    //}
    
    output.color.rgb += environmentColor.rgb * gMaterial.environmentCoefficient;

    return output;
}