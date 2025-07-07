#include "object3d.hlsli"
struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord.xy);
    
    output.color = textureColor;
    
    if (textureColor.a == 0.0)
    {
        discard;
    }
    if (output.color.a == 0.0f)
    {
        discard;
    }
    return output;
}