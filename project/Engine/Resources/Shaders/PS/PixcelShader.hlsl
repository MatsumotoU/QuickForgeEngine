#include "../ShaderStructs/Object3d.hlsli"
#include "../ShaderStructs/hlslTypeToCpp.h"

ConstantBuffer<PixcelOffset> gOffsetBuffer : register(b0);
Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

float32_t2 MosaicUV(float32_t2 uv, float32_t2 textureSize, float32_t2 mosaicSize)
{
    float32_t2 pixelCoord = uv * textureSize;
    pixelCoord = floor(pixelCoord / mosaicSize) * mosaicSize + mosaicSize * 0.5;
    return pixelCoord / textureSize;
}

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    float32_t2 textureSize = float32_t2(gOffsetBuffer.screenResolution.x, gOffsetBuffer.screenResolution.y);
    float32_t mosaicSize = gOffsetBuffer.pixcelSize;

    float32_t2 mosaicUV = MosaicUV(input.texcoord, textureSize, mosaicSize);

    output.color = gTexture.Sample(gSampler, mosaicUV);
    
    return output;
}