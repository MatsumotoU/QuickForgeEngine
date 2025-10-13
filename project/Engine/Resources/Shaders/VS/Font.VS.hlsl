#include "../ShaderStructs/Particle.hlsli"
#include "../ShaderStructs/hlslTypeToCpp.h"

StructuredBuffer<GlyphForGPU> gGlyph : register(t0);

struct VertexShaderInput
{
    float32_t4 position : POSITION0;
    float32_t4 color : COLOR0;
    float32_t2 texcoord : TEXCOORD0;
};

VertexShaderOutput main(VertexShaderInput input, uint32_t instanceId : SV_InstanceID)
{
    VertexShaderOutput output;
    output.position = mul(input.position, gGlyph[instanceId].WVP);
    
    // Create UV
    float2 uv;
    uv.x = lerp(gGlyph[instanceId].texCoords.x, gGlyph[instanceId].texCoords.z, input.texcoord.x);
    uv.y = lerp(gGlyph[instanceId].texCoords.y, gGlyph[instanceId].texCoords.w, input.texcoord.y);
    output.texcoord = uv;
    output.color = input.color;
    
    return output;
}