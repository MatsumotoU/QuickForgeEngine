#include "../ShaderStructs/Particle.hlsli"
#include "../ShaderStructs/hlslTypeToCpp.h"

ConstantBuffer<Material> gMaterial : register(b0);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    //output.color = gMaterial.color * input.color;
    output.color = input.color;
    
    if (output.color.a == 0.0f)
    {
        //output.color = float32_t4(1.0f, 0.0f, 0.0f, 1.0f);
        discard;
    }
    
    return output;
}