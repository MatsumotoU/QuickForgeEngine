#include "../ShaderStructs/Object3d.GBuffer.hlsli"
#include "../ShaderStructs/hlslTypeToCpp.h"

ConstantBuffer<ObjectTransformGpuData> gTransformationMatrix : register(b0);

VertexShaderOutput main(MeshVertexData input)
{
    VertexShaderOutput output;
    output.position = mul(input.position, gTransformationMatrix.WVP);
    output.texcoord = input.texcoord;
    output.normal = normalize(mul(input.normal, (float32_t3x3) gTransformationMatrix.World));
    output.worldPosition = mul(input.position, gTransformationMatrix.World).xyz;
    return output;
}