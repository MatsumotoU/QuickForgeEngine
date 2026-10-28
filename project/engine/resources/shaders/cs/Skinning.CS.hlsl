#include "../ShaderStructs/hlslTypeToCpp.h"
StructuredBuffer<SkinningPaletteEntry> gMatrixPalette : register(t0);
StructuredBuffer<SkinningVertex> gVertex : register(t1);
StructuredBuffer<SkinningVertexInfluence> gVertexInfluences : register(t2);
RWStructuredBuffer<SkinningVertex> gOutputVertex : register(u0);
ConstantBuffer<SkinningDispatchParameters> gSkinningInformation : register(b0);

[numthreads(8, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint32_t vertexIndex = DTid.x;
    if (vertexIndex >= gSkinningInformation.numVertices)
    {
        return;
    }
    
    SkinningVertex input = gVertex[vertexIndex];
    SkinningVertexInfluence influence = gVertexInfluences[vertexIndex];
    SkinningVertex skinned;
    skinned.texcoord = input.texcoord;
    skinned.position = float32_t4(0.0f, 0.0f, 0.0f, 0.0f);
    skinned.normal = float32_t3(0.0f, 0.0f, 0.0f);
    
    skinned.position = mul(input.position, gMatrixPalette[vertexIndex]) * influence.weight.x;
    skinned.position += mul(input.position, gMatrixPalette[vertexIndex]) * influence.weight.y;
    skinned.position += mul(input.position, gMatrixPalette[vertexIndex]) * influence.weight.z;
    skinned.position += mul(input.position, gMatrixPalette[vertexIndex]) * influence.weight.w;
    skinned.position.w = 1.0f;
    
    skinned.normal = mul(input.normal, (float32_t3x3) gMatrixPalette[vertexIndex]) * influence.weight.x;
    skinned.normal += mul(input.normal, (float32_t3x3) gMatrixPalette[vertexIndex]) * influence.weight.y;
    skinned.normal += mul(input.normal, (float32_t3x3) gMatrixPalette[vertexIndex]) * influence.weight.z;
    skinned.normal += mul(input.normal, (float32_t3x3) gMatrixPalette[vertexIndex]) * influence.weight.w;
    skinned.normal = normalize(skinned.normal);
    
    gOutputVertex[vertexIndex] = skinned;    
}