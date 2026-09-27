#include "../ShaderStructs/hlslTypeToCpp.h"

ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);

struct VertexShaderInput
{
    float32_t4 position : POSITION0;
    float32_t4 color : COLOR0;
};

struct VertexShaderOutput
{
    float32_t4 position : SV_Position;
    float32_t4 color : TEXCOORD0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    output.position = mul(input.position, gTransformationMatrix.WVP);
    output.color = input.color;
    return output;
}
