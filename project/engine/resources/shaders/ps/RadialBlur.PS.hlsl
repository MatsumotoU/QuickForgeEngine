#include "../ShaderStructs/Object3d.hlsli"
#include "../ShaderStructs/hlslTypeToCpp.h"

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    const float32_t2 center = float32_t2(0.5f, 0.5f);
    const int32_t kNumSamples = 10;
    const float32_t kBlurWidth = 0.01f;
    
    float32_t2 direction = input.texcoord - center;
    float32_t3 outputColor = float32_t3(0.0f, 0.0f, 0.0f);
    for (int32_t i = 0; i < kNumSamples; ++i)
    {
        float32_t t = float32_t(i) / float32_t(kNumSamples - 1);
        float32_t2 sampleCoord = input.texcoord - direction * t * kBlurWidth;
        outputColor += gTexture.Sample(gSampler, sampleCoord).rgb;
    }
    output.color.rgb *= rcp(kNumSamples);
    
    output.color = float32_t4(outputColor, 1.0f);
    return output;
}