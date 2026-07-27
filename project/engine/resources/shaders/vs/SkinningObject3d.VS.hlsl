#include "../ShaderStructs/hlslTypeToCpp.h"
#include "../ShaderStructs/Object3d.hlsli"

StructuredBuffer<Well> gMatrixPalette : register(t0);

struct VertexShaderInput {
    float32_t4 position : POSITION0;
    float32_t2 texcoord : TEXCOORD0;
    float32_t3 normal : NORMAL0;
    float32_t4 weight : BLENDWEIGHT0;
    int32_t4 index : BLENDINDICES0;
};

struct Skinned
{
    float32_t4 position;
    float32_t3 normal;
};

Skinned SkinningObject3d(VertexShaderInput input)
{
    Skinned output;
    output.position = float32_t4(0.0f, 0.0f, 0.0f, 0.0f);
    output.normal = float32_t3(0.0f, 0.0f, 0.0f);
    
    output.position = mul(input.position, gMatrixPalette[input.index.x]) * input.weight.x;
    output.position += mul(input.position, gMatrixPalette[input.index.y]) * input.weight.y;
    output.position += mul(input.position, gMatrixPalette[input.index.z]) * input.weight.z;
    output.position += mul(input.position, gMatrixPalette[input.index.w]) * input.weight.w;
    output.position.w = 1.0f;
    
    output.normal = mul(input.normal, (float32_t3x3) gMatrixPalette[input.index.x]) * input.weight.x;
    output.normal += mul(input.normal, (float32_t3x3) gMatrixPalette[input.index.y]) * input.weight.y;
    output.normal += mul(input.normal, (float32_t3x3) gMatrixPalette[input.index.z]) * input.weight.z;
    output.normal += mul(input.normal, (float32_t3x3) gMatrixPalette[input.index.w]) * input.weight.w;
    output.normal = normalize(output.normal);
    
    return output;
}

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    Skinned skinned = SkinningObject3d(input);
    output.position = mul(skinned.position, gTransformationMatrix.WVP);
    output.texcoord = input.texcoord;
    output.normal = normalize(mul(skinned.normal, (float32_t3x3)gTransformationMatrix.World));
    output.worldPosition = mul(skinned.position, gTransformationMatrix.World).xyz;
    return output;
}