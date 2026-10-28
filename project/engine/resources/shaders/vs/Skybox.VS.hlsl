#include "../ShaderStructs/Skybox.hlsli"
#include "../ShaderStructs/hlslTypeToCpp.h"

ConstantBuffer<ObjectTransformGpuData> gTransformationMatrix : register(b0);

VertexShaderOutput main(MeshVertexData input)
{
    VertexShaderOutput output;
    output.position = mul(input.position, gTransformationMatrix.WVP).xyww;
    output.texcoord = input.position.xyz;
    return output;
}