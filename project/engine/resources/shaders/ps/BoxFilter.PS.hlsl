#include "../ShaderStructs/Object3d.hlsli"
#include "../ShaderStructs/hlslTypeToCpp.h"

ConstantBuffer<BoxFilterOffset> gOffsetBuffer : register(b0);
Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

static const float kernel[9] = { 
    1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f,
    2.0f / 16.0f, 4.0f / 16.0f, 2.0f / 16.0f,
    1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f};

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
    
    
    
    
    
    return output;
}