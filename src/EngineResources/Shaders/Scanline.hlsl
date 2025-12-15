void mainImage( out float4 fragColor, in float2 fragCoord )
{
    float  division = 50.0;
    float  speed = 3.0;
    float4 color0 = float4(0.7, 0.7, 0.7, 1.0);
    float4 color1 = float4(1.0, 1.0, 1.0, 1.0);
    
    float2 uv = fragCoord / iResolution.xy;
    
    float pos = uv.y * division + iTime * speed;
    
    // 0 <= bit < 2.0f
    float bit = mod(pos, 2.0);
    float dodge = step(1.0, bit);
    ouput.color = color0 * (1.0f - dodge) + color1 * dodge;
}