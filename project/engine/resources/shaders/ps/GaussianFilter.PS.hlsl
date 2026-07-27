#include "../ShaderStructs/Object3d.hlsli"
#include "../ShaderStructs/hlslTypeToCpp.h"

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

static float32_t pi = 3.1415926535897932384626433832795f;

static const int2 offsets[9] =
{
    int2(-1, -1), int2(0, -1), int2(1, -1),
    int2(-1, 0), int2(0, 0), int2(1, 0),
    int2(-1, 1), int2(0, 1), int2(1, 1)
};

float gauss(float x, float y, float sigma)
{
    float exponent = -(x * x + y * y) / (2.0f * sigma * sigma);
    float denominator = 2.0f * pi * sigma * sigma;
    return exp(exponent) * rcp(denominator);
};

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    uint width;
    uint height;
    gTexture.GetDimensions(width, height);
    float2 texelSize = rcp(float2(width, height));

    float32_t weight = 0.0f;
    float32_t kernel[9];
    for (int i = 0; i < 9; ++i)
    {
        float2 offset = float2(offsets[i]);
        kernel[i] = gauss(offset.x, offset.y, 2.0f);
        weight += kernel[i];
    }

    float32_t4 color = float32_t4(0.0f, 0.0f, 0.0f, 0.0f);
    for (int i = 0; i < 9; ++i)
    {
        float2 offset = float2(offsets[i]) * texelSize;
        float32_t4 sample = gTexture.Sample(gSampler, input.texcoord + offset);
        color += sample * kernel[i];
    }

    output.color = color * rcp(weight);
    return output;
}