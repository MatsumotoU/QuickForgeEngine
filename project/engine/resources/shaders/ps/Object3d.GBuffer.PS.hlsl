#include "../ShaderStructs/Object3d.GBuffer.hlsli"
#include "../ShaderStructs/hlslTypeToCpp.h"

ConstantBuffer<Material> gMaterial : register(b0);
Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float32_t4 position : SV_Target0;
    float32_t4 normal : SV_Target1;
    float32_t4 color : SV_Target2;
};
PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float4 transformedUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float32_t4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    
    float alpha = gMaterial.color.a * textureColor.a;

    if (alpha == 0.0f)
    {
        discard;
    }
    
    output.position = float32_t4(input.worldPosition, 1.0f);
    output.normal = float32_t4(normalize(input.normal), 1.0f);
    output.color = float32_t4(gMaterial.color.rgb * textureColor.rgb, alpha);
    return output;
}
