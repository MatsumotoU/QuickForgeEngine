#include "../ShaderStructs/Object3d.hlsli"
#include "../ShaderStructs/hlslTypeToCpp.h"

ConstantBuffer<BoxFilterOffset> gOffsetBuffer : register(b0);
Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

static const float kernel[9] = { 
    1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f,
    1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f,
    1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f
};

static const int2 offsets[9] = {
    int2(-1, -1), int2(0, -1), int2(1, -1),
    int2(-1, 0),  int2(0, 0),  int2(1, 0),
    int2(-1, 1), int2(0, 1), int2(1, 1)
};

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float32_t4 color = float32_t4(0.0f, 0.0f, 0.0f, 0.0f);
    for (int i = 0; i < 9; ++i)
    {
        int2 offset = offsets[i];
        float32_t4 sample = gTexture.Sample(gSampler, input.texCoord + float2(offset) * gOffsetBuffer.offset);
        color += sample * kernel[i];
    }
    output.color = color;
    return output;
}