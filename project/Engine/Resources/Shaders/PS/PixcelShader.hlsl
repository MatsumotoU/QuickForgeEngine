#include "../ShaderStructs/Object3d.hlsli"
#include "../ShaderStructs/hlslTypeToCpp.h"

ConstantBuffer<PixcelOffset> gOffsetBuffer : register(b0);
Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

float rand(float2 co)
{
    return frac(sin(dot(co, float2(12.9898, 78.233))) * 43758.5453);
}

float2 FisheyeUV(float2 uv)
{
    float2 center = float2(0.5, 0.5);
    float2 delta = uv - center;
    float dist = length(delta);
    float strength = 0.05; 
    float r = dist;
    float theta = atan2(delta.y, delta.x);
    r = pow(r, 1.0 + strength);
    float2 newDelta = float2(cos(theta), sin(theta)) * r;
    return center + newDelta;
}

float Scanline(float y)
{
    float frequency = 400.0; 
    float intensity = 0.15;
    return 1.0 - intensity * (0.5 + 0.5 * sin(y * frequency));
}

float2 ChromaticUV(float2 uv, float offset)
{
    float2 dir = uv - float2(0.5, 0.5);
    return uv + dir * offset;
}

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    float2 textureSize = float2(gOffsetBuffer.screenResolution.x, gOffsetBuffer.screenResolution.y);

    float2 fisheyeUV = FisheyeUV(input.texcoord);

    float chromaOffset = 0.003;
    float4 color;
    color.r = gTexture.Sample(gSampler, ChromaticUV(fisheyeUV, +chromaOffset)).r;
    color.g = gTexture.Sample(gSampler, ChromaticUV(fisheyeUV, 0.0)).g;
    color.b = gTexture.Sample(gSampler, ChromaticUV(fisheyeUV, -chromaOffset)).b;
    color.a = 1.0;

    float scan = Scanline(fisheyeUV.y);

    float noise = rand(fisheyeUV * textureSize + gOffsetBuffer.time) * 0.03 - 0.015;
    
    color.rgb = color.rgb * scan + noise;

    output.color = saturate(color);

    return output;
}