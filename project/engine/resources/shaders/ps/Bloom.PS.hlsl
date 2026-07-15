#include "../ShaderStructs/Object3d.hlsli"
#include "../ShaderStructs/hlslTypeToCpp.h"

ConstantBuffer<BloomOffset> gOffsetBuffer : register(b0);
Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

float3 GetBrightPart(float3 color)
{
    float luminance = dot(color, float3(0.299f, 0.587f, 0.114f));
    return max(color * (luminance - gOffsetBuffer.threshold) / (1.0f - gOffsetBuffer.threshold), 0.0f);
}

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    float4 baseColor = gTexture.Sample(gSampler, input.texcoord);
    
    float2 offset = gOffsetBuffer.screenSize * 15.5f;
    
    float3 blurColor = float3(0.0f, 0.0f, 0.0f);

    blurColor += GetBrightPart(gTexture.Sample(gSampler, input.texcoord + float2(offset.x, offset.y)).rgb);
    blurColor += GetBrightPart(gTexture.Sample(gSampler, input.texcoord + float2(-offset.x, offset.y)).rgb);
    blurColor += GetBrightPart(gTexture.Sample(gSampler, input.texcoord + float2(offset.x, -offset.y)).rgb);
    blurColor += GetBrightPart(gTexture.Sample(gSampler, input.texcoord + float2(-offset.x, -offset.y)).rgb);
    
    blurColor *= 0.25f;
    
    float3 finalColor = baseColor.rgb + (blurColor * gOffsetBuffer.intensity);
    output.color.rgb = finalColor;
    output.color.a = baseColor.a;
    
    return output;
}