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
    
    float32_t weight = 0.0f;
    float32_t kernel[3][3];
    for (int32_t x = 0; x < 3; ++x){
        for (int32_t y = 0; y < 3; ++y){
            kernel[x][y] = gauss(offsets[y][0], offsets[y][1], 2.0f);
            weight += kernel[x][y];
        }
    }
    
    output.color.rgb = rcp(weight);
    
        return output;
}