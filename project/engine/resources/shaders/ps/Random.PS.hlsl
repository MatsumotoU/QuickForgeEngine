#include "../ShaderStructs/MiniShader.hlsli"
#include "../utility/Random.hlsli"

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    float32_t random = rand2dTo1d(input.texcoord);
    output.color = float32_t4(random, random, random, 1.0f);
    
    return output;
}