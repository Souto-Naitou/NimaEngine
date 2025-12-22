void mainImage(float4 fragColor, float2 fragCoord)
{
    float power = iTime * 2.0;
    float2 uv = fragCoord / iResolution.xy;
    float2 id = floor(uv * power);
    uv = (id + 0.5) / power;
    fragColor = texture(iChannel0, uv);
}
