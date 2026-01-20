#ifndef RUNTIME_DATA_HLSLI
#define RUNTIME_DATA_HLSLI

cbuffer RuntimeData : register(b0)
{
    float2 iResolution;
    float iTime;
    float iDeltaTime;
    
    uint iFrame;
    float3 _RuntimeDataPadding0;
};

#endif