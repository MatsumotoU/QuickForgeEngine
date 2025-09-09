#include "../ShaderStructs/Object3d.hlsli"

struct OffsetBuffer
{
    float32_t4 gOffset;
};

ConstantBuffer<OffsetBuffer> gOffsetBuffer : register(b0);
Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    float pixelSize = 0.001f;

    float noise = frac(sin(dot(input.texcoord.xy, float2(12.9898f, 78.233f))) * 43758.5453f);
    float noiseStrength = 0.05f;
    float noiseValue = (noise - 0.5f) * noiseStrength;

    float2 pixelatedUV = floor(input.texcoord.xy / pixelSize) * pixelSize;

    float2 chromaOffsetR = float2(-0.001f, 0.0f);
    float2 chromaOffsetG = float2(0.0f, 0.0f);
    float2 chromaOffsetB = float2(0.001f, 0.0f);

    float r = gTexture.Sample(gSampler, pixelatedUV + chromaOffsetR).r + noiseValue;
    float g = gTexture.Sample(gSampler, pixelatedUV + chromaOffsetG).g + noiseValue;
    float b = gTexture.Sample(gSampler, pixelatedUV + chromaOffsetB).b + noiseValue;
    float a = gTexture.Sample(gSampler, pixelatedUV).a;

    float scale = r * 0.29891f + g * 0.58661f + b * 0.11448f;

    float3 color = lerp(float3(r, g, b), float3(scale, scale, scale), gOffsetBuffer.gOffset.r);
    int screenHeight = 720;
    int yPixel = int(input.texcoord.y * screenHeight);
    float interlaceFactor = (yPixel % 2 == 0) ? 0.9f : 1.0f;
    color *= interlaceFactor;

    output.color = float4(color, a);
    return output;
}