#include "../ShaderStructs/Object3d.hlsli"
#include "../ShaderStructs/hlslTypeToCpp.h"

ConstantBuffer<OutlineOffset> gOffsetBuffer : register(b0);

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

Texture2D<float32_t> gDepthTexture : register(t1);
SamplerState gSamplerPoint : register(s1);

static const float horizontalKernel[3][3] =
{
    { -1.0f / 6.0f, 0.0f, 1.0f / 9.0f },
    { -1.0f / 6.0f, 0.0f, 1.0f / 9.0f },
    { -1.0f / 6.0f, 0.0f, 1.0f / 9.0f }
};

static const float verticalKernel[3][3] =
{
    { -1.0f / 6.0f, -1.0f / 6.0f, -1.0f / 6.0f },
    { 0.0f, 0.0f, 0.0f },
    { 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f }
};

static const int2 offsets[9] =
{
    int2(-1, -1), int2(0, -1), int2(1, -1),
    int2(-1, 0), int2(0, 0), int2(1, 0),
    int2(-1, 1), int2(0, 1), int2(1, 1)
};

float32_t Luminance(float32_t4 color)
{
    return dot(color.rgb, float3(0.299f, 0.587f, 0.114f));
}

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float32_t2 difference = float32_t2(0.0f, 0.0f);
    for(int32_t x = 0; x < 3; ++x)
    {
        for(int32_t y = 0; y < 3; ++y)
        {
            float32_t2 texCoordOffset = input.texcoord + offsets[y * 3 + x];
            float32_t ndcDepth = gDepthTexture.Sample(gSamplerPoint, texCoordOffset);
            float32_t4 viewSpacePos = mul(float32_t4(0.0f, 0.0f, ndcDepth, 1.0f), gOffsetBuffer.viewProjectionInverse);
            float32_t viewX = viewSpacePos.x * rcp(viewSpacePos.w);
            difference.x += viewX * horizontalKernel[x][y];
            difference.y += viewX * verticalKernel[x][y];
        }
    }
    float32_t weight = length(difference);
    weight = saturate(weight);
    
    output.color = float32_t4(weight, weight, weight, 1.0f);
    return output;
}