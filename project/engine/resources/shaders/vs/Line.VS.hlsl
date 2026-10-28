#include "../ShaderStructs/hlslTypeToCpp.h"

ConstantBuffer<ObjectTransformGpuData> gTransformationMatrix : register(b0);

struct VertexShaderOutput
{
    float32_t4 position : SV_Position;
    float32_t4 color : TEXCOORD0;
};

VertexShaderOutput main(LineVertex input)
{
    VertexShaderOutput output;
    output.position = mul(input.position, gTransformationMatrix.WVP);
    output.color = input.color;
    return output;
}
