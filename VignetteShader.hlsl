#include "object3d.hlsli"

struct OffsetBuffer
{
    float32_t2 screenResolution;
    float VignetteRadius;
    float VignetteSoftness;
    float VignetteIntensity;
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
    float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord.xy);
    float32_t2 uv = input.texcoord.xy - 0.5f;
    float distance = length(uv);
    float vignette = smoothstep(gOffsetBuffer.VignetteRadius, gOffsetBuffer.VignetteRadius + gOffsetBuffer.VignetteSoftness, distance);
    vignette = 1.0f - (vignette * gOffsetBuffer.VignetteIntensity);
    output.color.rgb = textureColor.rgb * vignette;
    output.color.a = textureColor.a;
    return output;
}