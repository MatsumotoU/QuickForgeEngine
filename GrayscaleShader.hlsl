#include "object3d.hlsli"

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
    float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord.xy);
    
    float scale = textureColor.r * 0.29891f + textureColor.g * 0.58661f +  textureColor.b * 0.11448f;
    output.color.r = lerp(textureColor.r, scale, gOffsetBuffer.gOffset.r);
    output.color.g = lerp(textureColor.g, scale, gOffsetBuffer.gOffset.r);
    output.color.b = lerp(textureColor.b, scale, gOffsetBuffer.gOffset.r);
    output.color.a = textureColor.a;
    
    return output;
}